
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


//-----------------------------------------------------------------------------------------
//Includes all external libs for the auth-fs

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <glib.h>

//#include "include/passthrough_helpers.h"
#include "include/storage.h"

//-----------------------------------------------------------------------------------------

// Storage DB file path
#define STORAGE_PATH "../db/storage.db"

// Database struct and stats
DB storagedb = NULL;



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
    printf("[open] %s\n", path);
    
    //1. Check context and file owner-------------------------------------------

    //get fuse context - which user requested this file
    struct fuse_context *ctx = fuse_get_context();
     
    uid_t uid_req; 
    gid_t gid_req;

    if (ctx != NULL) {
        uid_req = ctx -> uid;
        gid_req = ctx -> gid;
    }
 
    //get IDs of requester PID
    struct stat info_pid;
    //get file stat - whos the owner of the file
    stat(path, &info_pid);
       
    //get info about owner and requester
    struct passwd *pwd_owner = getpwuid(info_pid.st_uid);
    struct passwd *pwd_req   = getpwuid(uid_req); 
    struct group  *grp_owner = getgrgid(info_pid.st_gid);  

    if (pwd_owner != 0 && grp_owner != 0) {
            
        printf("[open] %s\nRequest   : |user=<%s><u_%d><g_%d>|\nFile info : |owner=<%s><u_%d><g_%d>)\n",path,pwd_req->pw_name,uid_req,gid_req,pwd_owner->pw_name,pwd_owner->pw_uid,pwd_owner->pw_gid);
    }

    //2. Process file opening---------------------------------------------------

	int res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
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
