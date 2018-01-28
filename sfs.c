 
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <fuse_common.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

#include "sfs.h"

char *storage;
int storage_fd;
static int inode_counter = 0;

int create_inode (struct inode * inode, FILE_TYPE ftype ){

  inode -> file_type = ftype;
  inode -> access_permission = 0777;
  inode -> file_size = 4 * BLK_SIZE;
  inode -> blks_in_use = 4;
  inode -> last_accessed = 0;
  inode -> last_modified = 0;
  inode -> inode_last_mod = time( NULL );
  //allocate blocks
  //

  return 0;
}

int create_file_entry (struct file_entry * fentry, const char * fname){

fentry -> inode_num = inode_counter = 0;
strcpy (fentry -> name, fname)
fentry -> all_files = (int **)malloc(sizeof(int*));
fentry -> all_files[0] = (int *)malloc(sizeof(int)* 5);
}



void 
struct inode *ret_inode(const char *path){
  int i;
  for(i=0; file_entries[i] != -1; i++){
    if(strcmp(path, file_entries[i]) == 0){
      return inode_entries[file_entries[i]->inode_num];
    }
  }
  return -1;
}

static int init_storage(){
  #if IN_MEM_STORE
    int fs_size = BLK_SIZE * NUM_OF_BLKS;
    storage = (char *)malloc(fs_size);

    if(storage == NULL){
      fprintf(stderr, "Memory not allocated\n");
      return -1;
    }
    else{
      memset(storage, 0, fs_size);
    }
    #else
      int abc;
    #endif
}

static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
	st->st_uid = 0; //getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = 0; //getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
  struct inode *inod = ret_inode(path);

	if ( strcmp( path, "/" ) == 0 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
  else if(inod->file_type == DIRECTORY){
    st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
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

  for(i=2; file_entry[i] != -1;i++){
    filler(buffer, file_entry[i]->file_name, NULL, 0, 0);
  }
  return 0;
  
}

static int do_mknod(const char *path, mode_t m, dev_t d){
  if(ret_inode(path) == -1){
    struct inode *inod = malloc(sizeof(struct inode));
    char* temp = path;
    char *dir_name = malloc(32);
    int i = 1;
    dir_name[0] = "/";
    if(temp[0] == "/"){
      temp++
      while(temp[0] != "/" && temp[0] != NULL){
        dir_name[i] = temp[0];
        temp++;i++;
      }

      if((inod = ret_inode(dir_name)) != -1){
        if(inod->file_type == DIRECTORY){

        }
      }

      

  }
}


static int do_read (const char *path, char *buffer, size_t size, off_t off, struct fuse_file_info *fi){
  struct stat *st;
  do_getattr(path, st);
  int fd, restat;

  if(st->st_mode == S_IFDIR)
    return -ENOENT;
  else{
    printf("Read operation...\n");

    int size = sizeof(file_entry)/sizeof(file_entry[0]);
    int i;

    for(i=0; i<size; i++){
      if(strcmp(path, file_entry[i]->file_name) == 0){
        //TODO : read the data from the file
        if(file_entry > 1){
          struct inode *inode_entry = inode_entries[file_entry[i]->inode_num];
          
          char* content = malloc((inode_entry->blks_in_use * BLK_SIZE)+1);

          int offset = 0, i;

          for(i=0; i<inode_entry->blks_in_use; i++){
            memcpy(&content[offset], inode_entry->block_addrs[i], BLK_SIZE); //not sure if it works
            offset+=BLK_SIZE;
          }

          memcpy(buffer, content+off, size);
          free(content);
          return strlen(content)-off;

        }
      }
    }
    return -ENOENT;
    }
  }

static int do_open(const char *path, struct fuse_file_info *fi){
  
  if(strcmp(path, "/") == 0)
    return -ENOENT;
  
  int i;

      
  if((fi->flags & O_ACCMODE) == O_RDONLY){
    return 0;
  }
  if((fi->flags & O_ACCMODE) == O_CREAT){
    if(do_mknod(path, 0644, 0) == 0)
      return 0;
    return -EACCES;
  }
  
  return -EACCES;
}

 

static int do_write(const char *path, const char *buffer, size_t size, off_t off, struct fuse_file_info *fi){
  struct stat *st;
  do_getattr(path, st);
  int fd, restat;

  if(st->st_mode == S_IFDIR)
    return -ENOENT;
  else{
    printf("Writing to a file")
    
    int size = sizeof(file_entry)/sizeof(file_entry[0]);
    int i;

    for(i=0; i<size; i++){
      if(strcmp(path, file_entry[i]->file_name) == 0){
        struct inode i = inode_entries[file_entry[i]->inode_num];
        int offset = 0, i;

        for(i=0; i<inode_entry->blks_in_use; i++){
            memcpy( inode_entry->block_addrs[i], &buf[offset], BLK_SIZE); //not sure if it works
            offset+=BLK_SIZE;
          }
      return strlen(buf) - off;
  }
  return -ENOENT;
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