
#define FUSE_USE_VERSION 31
  
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#ifdef __FreeBSD__
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif


//---------------------------------------------------------------------------
//Includes all external libs for the auth-fs

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <glib.h>
#include <json-c/json.h>

//#include "include/passthrough_helpers.h"
#include "include/storage.h"
#include "include/fs-tools.h"
#include "include/mail.h"
#include "include/auth-api.h"

//---------------------------------------------------------------------------

// Storage DB file path
//#define STORAGE_PATH "../db/storage.db"
#define STORAGE_PATH "/home/devzizu/Desktop/Computer-Systems-Security/tp3/filesystem-auth-operations/db/storage.db"
// Generated code standard size
#define GENERATED_CODE_SIZE 10
#define LIMIT_TIME_CODE_VALID 30

// Database struct and stats
DB storagedb = NULL;

//-----------------------------------------------------------------------------------------

char* process_get_code_confirmed(char* code) {

    char *res = NULL;

    char* get_result = curl_get_code_confirmed(code);

    // got response
    if (get_result) {

        struct json_object *parsed_json;
        struct json_object *result_field;

        parsed_json = json_tokener_parse(get_result);

        //{"result":"Not Found"}
        json_object_object_get_ex(parsed_json, "result", &result_field);
        res = strdup(json_object_get_string(result_field));
    }

    return res;
}

//-----------------------------------------------------------------------------------------

static void *xmp_init(struct fuse_conn_info *conn,
		      struct fuse_config *cfg)
{
    printf("[init] ...\n");
	(void) conn;
	cfg->use_ino = 1;

	cfg->entry_timeout = 0;
	cfg->attr_timeout = 0;
	cfg->negative_timeout = 0;

	return NULL;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
    printf("[readlink] %s\n", path);
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf,
		       struct fuse_file_info *fi)
{
    

    printf("[getattr] %s\n", path);
    (void) fi;
	int res;

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi,
		       enum fuse_readdir_flags flags)
{
    printf("[readir] %s\n", path);
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
	(void) flags;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    printf("[mkdir] %s\n", path);
	
    int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
    printf("[rmdir] %s\n", path);
	
    int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode,
		     struct fuse_file_info *fi)
{
    printf("[create] %s\n", path);
	
    (void) fi;
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid,
		     struct fuse_file_info *fi)
{
    printf("[chown] %s\n", path);
	
    (void) fi;
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char *path, mode_t mode,
		      struct fuse_file_info *fi)
{
    printf("[create] %s\n", path);

	int res;

	res = open(path, fi->flags, mode);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
    printf("[open] path = %s\n", path);
    
    //1. Check context and file owner-------------------------------------------

    //get fuse context - which user requested this file
    struct fuse_context *ctx = fuse_get_context();
     
    uid_t uid_req; 

    if (ctx != NULL) {
        uid_req = ctx -> uid;
    }
 
    //get IDs of requester PID
    struct stat info_pid;
    //get file stat - whos the owner of the file
    stat(path, &info_pid);
       
    //get info about the requester
    struct passwd *pwd_requester = getpwuid(uid_req); 

    printf("-> User (context) :: <uid-%d> <name-%s>\n", uid_req, pwd_requester->pw_name);
    
    //get info about the owner of the file
    struct passwd pwent;
    struct passwd *pwentp;
    char buf[1024];

    getpwuid_r(info_pid.st_uid, &pwent, buf, sizeof buf, &pwentp);
    printf("-> Owner (of path): <name-%s> <uid-%d>\n", pwent.pw_name, info_pid.st_uid);
    
    int blocked = 1;

    if (!strcmp(pwent.pw_name, pwd_requester->pw_name)) {
        
        printf("I'm the owner!\n");

        //its me, im the owner
        //no need to check storage.db
    
        blocked = 0;

    } else {
        
        blocked = 1;

        // check if storage was modified
        
        if (has_storage_been_modified(storagedb)) {     
        
            printf("Storage was modified!\n");
            update_storage_database(storagedb);
        
        } else
            printf("Storage is the same!\n");
        
        //neet to contact the owner, if contact available
        
        if (has_contact(storagedb, pwent.pw_name)) {
        
            char* ownerContact = get_contact(storagedb, pwent.pw_name);

            printf("Contact found for owner %s, contact %s\n", pwent.pw_name, ownerContact);
    
            // generate code
            
            char* randomGeneratedCode = generate_random_code(GENERATED_CODE_SIZE);
            
            printf("Generated code: %s\n", randomGeneratedCode);
            
            // send email
            
            char* email = strdup(ownerContact);
            char* owner = strdup(pwent.pw_name);
            char* user  = strdup(pwd_requester->pw_name);
            char* file  = strdup(path);
            char* code  = strdup(randomGeneratedCode);
            
            send_code_validation_email(email, owner, user, file, code);
    
            // wait until code verified
            // ...
            
            struct timespec t = { 1/*seconds*/, 0/*nanoseconds*/};
        
            int sec = 0;
            int codeConfirmed = -1;

            while (sec < LIMIT_TIME_CODE_VALID) {

                printf("%d seconds passed...\n", sec+1);
                nanosleep(&t,NULL);
                fflush(stdout); //see below

                //call api
                char* res = process_get_code_confirmed(code);

                if (res) {
                    printf("call api result = %s\n", res);
                    if (!strcmp("authorize", res)) {
                        codeConfirmed = 1;
                        break;
                    } else if (!strcmp("deny", res)){
                        codeConfirmed = 0;
                        break;
                    }
                }

                sec++;
            }
            
            if (codeConfirmed == 1) {
                printf("OK access granted...\n");
                blocked = 1;
            } else if (codeConfirmed == 0) {
                printf("Access denied by owner...\n");
                blocked = 0;
            } else {
                blocked = 1;
                printf("Time expired, access not granted...\n");
            }
            
        } else {
            
            blocked = 1;

            //no contact, block access
            printf("Owner has no contact, blocking access!\n");
        }
    }


    //2. Process file opening--------------------------------------------------- 
        
    //FIXME 

    if (blocked == 0) {
    
        int res = open(path, fi->flags);
        if (res == -1)
            return -errno;

        fi->fh = res;
        
        return 0;
    }

	return -errno;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    printf("[read] %s\n", path);
	
    int fd;
	int res;

	if(fi == NULL)
		fd = open(path, O_RDONLY);
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	if(fi == NULL)
		close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
    printf("[write] %s\n", path);
	
    int fd;
	int res;

	(void) fi;
	if(fi == NULL)
		fd = open(path, O_WRONLY);
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	if(fi == NULL)
		close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
    
    printf("[statfs] %s\n", path);

    //uid_t = fuse_get_context() -> uid;
    //gid_t = fuse_get_context() -> gid;

	int res;
	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
    printf("[access] %s\n", path);
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2],
		       struct fuse_file_info *fi)
{
    printf("[utimens] %s\n", path);

	(void) fi;
	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int xmp_release(const char *path, struct fuse_file_info *fi)
{

    printf("[release] %s\n", path);

	(void) path;
	close(fi->fh);
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
    printf("[fsync] %s\n", path);
	
    /* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
    printf("[getxattr] %s\n", path);
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
    printf("[listxattr] %s\n", path);
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}
#endif /* HAVE_SETXATTR */

static const struct fuse_operations xmp_oper = {
	.init           = xmp_init,
	.getattr	= xmp_getattr,
    .access     = xmp_access,
    .readlink   = xmp_readlink,
    .readdir	= xmp_readdir,
	.mkdir		= xmp_mkdir,
	.rmdir		= xmp_rmdir,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
#ifdef HAVE_UTIMENSAT
	.utimens	= xmp_utimens,
#endif
	.open		= xmp_open,
	.create 	= xmp_create,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
    .release	= xmp_release,
    .fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr
#endif
};

int main(int argc, char *argv[])
{

    //load contact storage database
    storagedb = malloc(sizeof(DB));
    load_contact_database(storagedb, STORAGE_PATH);
   
    //display contents
    print_contact_database(storagedb);
    
    //run filesystem
    umask(0);

	return fuse_main(argc, argv, &xmp_oper, NULL);    
}
