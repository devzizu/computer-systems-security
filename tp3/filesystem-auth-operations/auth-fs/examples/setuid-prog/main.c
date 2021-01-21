
#include <stdio.h>
#include <zconf.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

#define STORAGE "storage.db"

int main(int argc, char* argv[]) {
        
    //user running program
    uid_t uid  = getuid();
    uid_t euid = geteuid(); 
    
    //owner of file
    struct stat info;
    stat(STORAGE, &info);
    struct passwd *pw = getpwuid(info.st_uid);
    struct group  *gr = getgrgid(info.st_gid);

    printf("user running : uid=%d,euid=%d\n", uid, euid);
    printf("file owner   : name=%s, grou=%s", pw->pw_name, gr->gr_name);

    return 0;
}
