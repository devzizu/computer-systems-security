
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

#define STORAGE "storage.db"

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

    int fd = open(STORAGE, O_WRONLY | O_APPEND);
    
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

int main (void) {
  
    ruid = getuid ();
    euid = geteuid ();
    exec_undo_setuid ();
    
    // update contact ? new contact ?
    
    char *user, *contact;

    printf("username: ");
    user = fget_line();
 
    printf("new contact: ");
    contact = fget_line();
   
    printf("user: <%s> <%s>\n", user, contact);

    update_storage(user, contact);

    printf("file updated...\n");

    return 0;
}
