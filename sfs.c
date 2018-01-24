 
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
	// GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
	// 		st_uid: 	The user ID of the file’s owner.
	//		st_gid: 	The group ID of the file.
	//		st_atime: 	This is the last access time for the file.
	//		st_mtime: 	This is the time of the last modification to the contents of the file.
	//		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
	//		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon 
	//						as no process still holds it open. Symbolic links are not counted in the total.
	//		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.
	
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
	if ( strcmp( path, "/" ) == 0 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
	else
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	
	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
  printf("\nReading the directory  %s\n", path);

  DIR *dir;
  struct dirent *entry;
  struct stat *st;
  
  if((dir = opendir(path))!=NULL){
      while((entry = readdir(dir)) == 0){
        do_getattr(entry->d_name, st);
        filler(buffer, entry->d_name, st, 0, FUSE_FILL_DIR_PLUS);
        printf("%s\n", entry->d_name);
      }
  return 0;
  }

  else
    return -1;

}

static int do_read (const char *path, char *buffer, size_t size, off_t off, struct fuse_file_info *fi){
  struct stat *st;
  do_getattr(path, st);
  int fd, restat;

  if(st->st_mode == S_IFDIR)
    return -1;
  else{
    if((fd = open(path, O_RDONLY))<0)
      return -1;
    
    else{
      lseek(fd, off, 0);
      fi->fh = fd;
      while((restat = read(fd, buffer, size))>0);
      buffer[size] = 0x00;
      return strlen(buffer) - off;
    }
  }
}


static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &operations, NULL );
}