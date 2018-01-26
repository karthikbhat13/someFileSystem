 
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "sfs.h"


static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
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

  if(strcmp(path, "/") != 0)
    return -ENOENT;
  
  int i;

  for(i=2; dir_entries[i] != -1;i++){
    filler(buffer, dir_entries[i]->file_name, NULL, 0, 0);
  }
  return 0;
}


static int do_read (const char *path, char *buffer, size_t size, off_t off, struct fuse_file_info *fi){
  struct stat *st;
  do_getattr(path, st);
  int fd, restat;

  if(st->st_mode == S_IFDIR)
    return -ENOENT;
  else{
    printf("Read operation...\n");

    int size = sizeof(dir_entries)/sizeof(dir_entries[0]);
    int i;

    for(i=0; i<size; i++){
      if(strcmp(path, dir_entries[i]->file_name) == 0){
        //TODO : read the data from the file
        if(dir_entries > 1){
          struct inode *inode_entry = inode_entries[dir_entries[i]->inode_num];
          
          char* content = malloc((inode_entry->blks_in_use * BLK_SIZE)+1);

          int offset = 0, i;

          for(i=0; i<inode_entry->blks_in_use; i++){
            memcpy(&content[offset], inode_entry->block_addrs[i], BLK_SIZE); //not sure if it works
            offset+=BLK_SIZE;
          }

          memcpy(buffer, content+off, size);

          return strlen(content-off);

        }
      }
    }
    return -ENOENT;


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