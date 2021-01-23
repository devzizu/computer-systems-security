
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <zconf.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

#include "../auth-fs/include/global.h"

//-------------------------------------------------------------------------

static uid_t euid, ruid;

//-------------------------------------------------------------------------

void exec_do_setuid (void) {

    int status;

	#ifdef _POSIX_SAVED_IDS
        status = seteuid (euid);
    #else
        status = setreuid (ruid, euid);
    #endif

    if (status < 0) {
        
        fprintf (stderr, "Couldn't set uid.\n");
        exit (status);
    }
}

//-------------------------------------------------------------------------

void exec_undo_setuid (void) {
  
    int status;

    #ifdef _POSIX_SAVED_IDS
        status = seteuid (ruid);
    #else
        status = setreuid (euid, ruid);
    #endif
        
    if (status < 0) {
    
        fprintf (stderr, "Couldn't set uid.\n");
        exit (status);
    }
}

//-------------------------------------------------------------------------

int update_storage(char* user, char* contact) {
	
    //printf("(process) running program as real uid = %d, eff. uid = %d\n", ruid, euid);
    
    //printf("(...) setting up IDs");
    exec_do_setuid ();
    //printf("(process) running now as real uid = %d, eff. uid = %d\n", ruid, euid);
    
    //--------------------------------------------------------------------------------
    
    char buffer[500];

    sprintf(buffer, "%s::%s\n", user, contact);

    int fd = open(STORAGE_PATH_UPDCT, O_WRONLY | O_APPEND);
   
    write(fd, buffer, strlen(buffer));

    close(fd);

    //--------------------------------------------------------------------------------
    
    //printf("(...) reverting IDs");
    exec_undo_setuid();
    //printf("(process) running now as real uid = %d, eff. uid = %d\n", ruid, euid);

    return 0;
}

char* fget_line() {
    
    char line[256];
    int i;
    
    char* r = fgets(line, sizeof(line), stdin);

    line[strcspn(line, "\n")] = 0;

    if (!r) 
        return NULL;

    return strdup(line);
}

int validate_username(int real_uid, char* modify_user) {

    struct passwd pwent;
    struct passwd *pwentp;
    char buf[1024];

    getpwuid_r(real_uid, &pwent, buf, sizeof buf, &pwentp);
    
    //check if user exists
    if ((strlen(modify_user) == 0) || (getpwnam(modify_user) == NULL))
        return -1;

    //if he is root or is trying to change hes info, then OK
    if (!strcmp(modify_user, pwent.pw_name) || !strcmp(pwent.pw_name, STORAGE_ROOT_USER))
            return 1;
    
    return 0;
}

int validate_contact_email(char* email) {

    char domain[100];
    char username[100];
    int s = sscanf(email, "%[_a-zA-Z0-9.]@%[_a-zA-Z0-9.]", username, domain);

    return s == 2 ? 1 : 0;
}

int main (void) {
  
    ruid = getuid ();
    euid = geteuid ();
    exec_undo_setuid ();
    
    // update contact ? new contact ?
    
    char *user, *contact;

    printf("username: ");
    user = fget_line();

    int v = validate_username(ruid, user);
    if (!v) {
        printf("you are not able to modify %s account information...\n", user);
        return -1;
    } else if (v == -1) {
        printf("user %s account doesn't exist...\n", user);
        return -1;
    }

    printf("new contact: ");
    contact = fget_line();
   
    if (validate_contact_email(contact) == 0) {
        printf("please check the correct syntax for contact (email) field...\n", user);
        return -1;
    }

    printf("(after valid update) user: <%s> <%s>\n", user, contact);

    update_storage(user, contact);

    return 0;
}
