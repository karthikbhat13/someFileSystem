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

#include "bla.h"


#define FILE_SYSTEM_START_ADDRESS storage
#define DIRECTORY_STRUCTURE_0FFSET 0 
#define DIRECTORY_STRUCTURE_START_ADDRESS FILE_SYSTEM_START_ADDRESS + DIRECTORY_STRUCTURE_0FFSET
#define FILE_START_ADDRESS_OFFSET BLK_SIZE * 1024	//1MB for the directory structure
#define FILE_START_ADDRESS  FILE_SYSTEM_START_ADDRESS + FILE_START_ADDRESS_OFFSET


extern char *storage;
int storage_fd;
static int inode_counter = 0;


struct file_entry * root;

int resize_all_files (struct file_entry * fentry){
		
	fentry -> all_files = realloc (fentry -> all_files, fentry -> no_files + 5);


}

int create_inode (struct inode * inode, enum FILE_TYPE ftype ){

  inode -> file_type = ftype;
  inode -> access_permission = 0777;
  inode -> file_size = 4 * BLK_SIZE;
  inode -> blks_in_use = 4;
  inode -> last_accessed = 0;
  inode -> last_modified = 0;
  inode -> inode_last_mod = time( NULL );
  inode_counter ++;
  //allocate blocks
  //

  return 0;
}

int create_file_entry (struct file_entry * fentry, const char * fname, enum FILE_TYPE file){

	int i;
	fentry -> inode_num = inode_counter;//check
	strcpy (fentry->file_name, fname);
	
	//only for directories
	if (file == DIRECTORY){
		fentry -> all_files = (struct file_entry **)malloc(sizeof(struct file_entry) * 5);
		for (i = 0; i < 5; i++)
			fentry -> all_files[i] = (struct file_entry *)malloc(sizeof(struct file_entry));

		fentry -> no_files = 0;
		fentry -> max_file_count;
	}
}


//fix
int ret_inode(const char *path){
  int i;
  for(i=0; file_entries[i] != NULL; i++){
    if(strcmp(path, file_entries[i]->file_name) == 0){
      return file_entries[i]->inode_num;
    }
  }
  return -1;
}

static int init_storage(){
  #if IN_MEM_STORE
    int fs_size = BLK_SIZE * NUM_OF_BLKS;
    storage = (char *)malloc(fs_size);
    //allocate from storage chunk (fix)
    inode_entries =(struct inode *) malloc (5 * sizeof(struct inode);
    //create  a root directory
    //inode table memory allocation initially
    //
    //
    //
    // 
    //
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
	int inode_num;
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
	st->st_uid = 0; //getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = 0; //getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
  	if (inode_num = ret_inode(path) == -1)
		fprintf (stderr, "invalid path");

	if ( strcmp( path, "/" ) == 0 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
  else if(inode_entries[inode_num].file_type == DIRECTORY){
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

/*static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
  printf("\nReading the directory  %s\n", path);

  if(strcmp(path, "/") != 0)
    return -ENOENT;
  
  int i;
  filler(buffer, ".", NULL, 0, 0);
  filler(buffer, "..", NULL, 0, 0);
  /*
  for(i=2; file_entries[i] != NULL;i++){
    filler(buffer, file_entries[i]->file_name, NULL, 0, 0);
  }
  
  return 0;
  
}*/

//bugs present
static int do_mknod(const char *path, mode_t mode, dev_t dev){

  //char * test_string = strdup (path);
  /*if(ret_inode(path) == NULL){
    struct inode *inod = malloc(sizeof(struct inode));
    char* temp = path;
    char *dir_name = malloc(32);
    int i = 1;
    dir_name[0] = '/';
    if(temp[0] == '/'){
      temp++;
      while(temp[0] != '/' && temp[0] != NULL){
        dir_name[i] = temp[0];
        temp++;i++;
      }

      if((inod = ret_inode(dir_name)) != -1){
        if(inod->file_type == DIRECTORY){

        }
      }*/
	
	//if (inod = ret_inode () != NULL && inod.file_type =  )
	
	char * delim = "/";
	int found =0;
	int i;
	char *temp, *temp2;

	struct file_entry * new_entry;
	struct file_entry * traverse = root;
	struct inode * new_inode;

	//maybe not needed
	//if (path[0] = '/'){
		temp = strdup (path + 1);
		
		
		while (1){
			
			temp2 = strsep(&temp, delim);

			//work
			if (temp != NULL){


				for (i= 0; i < traverse -> no_files; i++){
					
					if (strcmp (traverse -> all_files[i][0].file_name, temp) == 0 && inode_entries[traverse -> all_files[i][0].inode_num].file_type == DIRECTORY){//also check if it is a directory, inode nmber is required
						found = 1;
						traverse = (traverse -> all_files[i]);					
						
					}
					
						
							
				
				}
				
				if (found == 0){
					
					//path not found error
					return 1;

					
				}
			 

			}
			
		}

		if (traverse == root){
			
			//assign the memory from the filesytem for inode and file strucutre entry
			//new_entry, new_inode
			
			create_inode (new_inode, REGULAR);
			create_file_entry (new_entry, temp2, REGULAR);
			
			if (traverse -> no_files == traverse -> max_file_count)
				resize_all_files (traverse);
			
			traverse -> all_files[traverse -> no_files]  = new_entry;
			traverse -> no_files++;
						
			
		}

			

		

	

      
    return 0;
}

/*do_mkdir (const char * path, mode_t mode){



	
	char * delim = "/";
	int found =0;
	int i=0;
	char *temp, *temp2;

	struct file_entry * new_entry;
	struct file_entry * traverse = root;
	struct inode * new_inode;

	//maybe not needed
	//if (path[0] = '/'){
		temp = strdup (path + 1);
		
		
		while (1){
			
			temp2 = strsep(&temp, delim);

			//work
			if (temp != NULL){


				for (i; i < traverse -> no_files; i++){
					
					if (strcmp (traverse -> all_files[0][i] -> file_name, temp) == 0 && traverse -> all_files[0][i] -> i){//also check if it is a directory, inode nmber is required
						
						found = 1;
						traverse = traverse -> all_files[0][i];					
						
					}
					
						
						
				
				}
				
				if (found == 0){
					
					//path not found error
					return 1;

					
				}
			 

			}
			
			else{
				if (traverse == root){
					
					//assign the memory from the filesytem for inode and file strucutre entry
					//new_entry, new_inode
					create_inode (new_inod, DIRECTORY);
					create_file_entry (new_entry, temp2, DIRECTORY);					
					
				}

			}
			

		}
		
	
	

      
    return 0;




}*/

/*static int do_read (const char *path, char *buffer, size_t size, off_t off, struct fuse_file_info *fi){
  struct stat *st;
  do_getattr(path, st);

  if(st->st_mode == S_IFDIR)
    return -ENOENT;
  else{
    printf("Read operation...\n");

    int size = sizeof(file_entries)/sizeof(file_entries[0]);
    int i;

    for(i=0; i<size; i++){
      if(strcmp(path, file_entries[i]->file_name) == 0){
        //TODO : read the data from the file
        if(i > 1){
          struct inode *inode_entry = inode_entries[file_entries[i]->inode_num];
          
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
  }*/

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

 

/*static int do_write(const char *path, const char *buffer, size_t size, off_t off, struct fuse_file_info *fi){
  struct stat *st;
  do_getattr(path, st);
  int fd, restat;

  if(st->st_mode == S_IFDIR)
    return -ENOENT;
  else{
    printf("Writing to a file");
    
    int size = sizeof(file_entries)/sizeof(file_entries[0]);
    int i;

    for(i=0; i<size; i++){
      if(strcmp(path, file_entries[i]->file_name) == 0){
        struct inode *inode_entry = inode_entries[file_entries[i]->inode_num];
        int offset = 0, j;
        
        for(j=0; j<inode_entry->blks_in_use; j++){
            memcpy( inode_entry->block_addrs[j], &buffer[offset], BLK_SIZE); //not sure if it works
            offset+=BLK_SIZE;
          }
      return strlen(buffer) - off;
  }
  return -ENOENT;
}
  }
}*/


static struct fuse_operations operations = {
    .getattr	= do_getattr,
    //.readdir	= do_readdir,
    //.read		= do_read,
};

int main(int argc, char *argv[])
{
	return fuse_main( argc, argv, &operations, NULL);
}
