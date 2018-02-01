 
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

//allocate the storage for directory structure which is file_entries 
// May be we can keep offset for each partition.
// Like dir_offset for file_entries
// inode_offset for inode_entries which is followed by data
int dir_offset = 0;   //Basically space allocated for directory structure is 1024 * sizeof(file_entry)
int inode_offset = 128;


#define FILE_SYSTEM_START_ADDRESS storage
#define DIRECTORY_STRUCTURE_0FFSET 0 
#define DIRECTORY_STRUCTURE_START_ADDRESS FILE_SYSTEM_START_ADDRESS + DIRECTORY_STRUCTURE_0FFSET
#define FILE_START_ADDRESS_OFFSET BLK_SIZE * 1024	//1MB for the directory structure
#define FILE_START_ADDRESS  FILE_SYSTEM_START_ADDRESS + FILE_START_ADDRESS_OFFSET


struct file_entry * root;

int resize_all_files (struct file_entry * fentry){
		
	fentry -> all_files = realloc (fentry -> all_files, sizeof(struct file_entry) * fentry -> no_files + 5);
  fentry -> max_file_count += 5;

}

int create_inode (struct inode * inod, enum FILE_TYPE ftype ){

  printf("Creating an inode entry\n\n");
  fflush(stdout);

  inod = malloc(sizeof(struct inode));


  inod -> file_type = ftype;
  inod -> access_permission = 0777;
  inod -> file_size = (BLKS_PER_INODE - 1) * BLK_SIZE;
  inod -> blks_in_use = BLKS_PER_INODE;
  inod -> last_accessed = 0;
  inod -> last_modified = 0;
  inod -> inode_last_mod = time( NULL );

  memcpy(&storage[inode_offset * BLK_SIZE], inod, BLK_SIZE);
  int i;
  for(i=1; i<BLKS_PER_INODE; i++){
    inod -> block_addrs[i] = &storage[inode_offset * (BLK_SIZE + i)];
    
  }
  inode_offset += BLKS_PER_INODE;

  

//PROBLEM HERE
  inode_entries[inode_counter++] = *inod;

  printf("inode 0 %d\n\n", inode_entries[0].file_type);
  return 0;
}

int create_file_entry (struct file_entry * fentry, const char * fname, enum FILE_TYPE file){

	int i;
  printf("Creating a file entry\n\n");
  fflush(stdout);

  fentry = malloc(sizeof(struct file_entry));
	fentry -> inode_num = inode_counter;//check
	strcpy (fentry->file_name, fname);
	
	//only for directories
	if (file == DIRECTORY){
		fentry -> all_files = (struct file_entry **)malloc(sizeof(struct file_entry) * 5);
		for (i = 0; i < 5; i++)
			fentry -> all_files[i] = (struct file_entry *)malloc(sizeof(struct file_entry));

		fentry -> no_files = 0;
		fentry -> max_file_count = 5;
	}
  memcpy(&storage[dir_offset  * BLK_SIZE], fentry, BLK_SIZE);


  file_entries[dir_offset++] = fentry;
   printf("inode 0 %s\n\n", file_entries[0]->file_name);
  return 0;
}


//fix
int ret_inode(const char *path){
  int i;
  for(i=0; i<5; i++){
    if(strcmp(path, file_entries[i]->file_name) == 0){
      return file_entries[i]->inode_num;
    }
  }
  return -1;
}




//bugs present
static int do_mknod(const char *path, mode_t mode, dev_t dev){

	char * delim = "/";
	int found =0;
	int i;
	char *temp, *temp2;

	struct file_entry * new_entry;
	struct file_entry * traverse = root;
	struct inode * new_inode;

	//maybe not needed
	//if (path[0] = '/')
  if(strcmp(path, "/") == 0){
    create_inode (new_inode, REGULAR);
    create_file_entry (new_entry, path, REGULAR);
    if(root -> no_files == root -> max_file_count)
      resize_all_files(root);
    root -> all_files[root ->no_files++] = new_entry;

  }
  else{
		temp = strdup(path + 1);
		while (1){
			temp2 = strsep(&temp, delim);
      printf("Stuck in mknod while %s path", temp2);
			//work
			if (temp2 != NULL){


				for (i= 0; i < traverse -> no_files; i++){
					found = 0;
					if (strcmp (traverse -> all_files[i][0].file_name, temp) == 0 && inode_entries[traverse -> all_files[i][0].inode_num].file_type == DIRECTORY){//also check if it is a directory, inode nmber is required
						found = 1;
						traverse = (traverse -> all_files[i]);					
					}
				}
				
				if (found == 0){
					
					//path not found error
					return -1;
				}
        else
          break;
			}
      else
        break;
			
		}
  
    
    //assign the memory from the filesytem for inode and file strucutre entry
    //new_entry, new_inode
    
    create_inode (new_inode, REGULAR);
    create_file_entry (new_entry, path, REGULAR);
    
    if (traverse -> no_files == traverse -> max_file_count)
      resize_all_files (traverse);
    
    traverse -> all_files[traverse -> no_files]  = new_entry;
    traverse -> no_files++;
  }
    return 0;
}




static int init_storage(){

  printf("Storage is allocating\n\n");
  fflush(stdout);

  int fs_size = BLK_SIZE * NUM_OF_BLKS;
  storage = (char *)malloc(fs_size);
  printf("%d\n", sizeof(storage));
  if(storage == NULL){
    fprintf(stderr, "Memory not allocated\n");
    return -1;
  }
  else{
    printf("Memory has been allocated\n");
    memset(storage, 0, fs_size);
  }
  int i;
  //allocate from storage chunk (fix)
  inode_entries =(struct inode *) malloc (5 * sizeof(struct inode));
  file_entries = (struct file_entry **) malloc(5 * sizeof(struct file_entry *));
  for(i=0; i<5; i++)
    file_entries[i] = (struct file_entry *)malloc(sizeof(struct file_entry));
  create_file_entry(root, "/", DIRECTORY);
  create_inode(root, DIRECTORY);

  
  for(i=0; i<5; i++){
    printf("MKNOD!!!");
    char temp = i+65;
    do_mknod("/"+temp, S_IFREG, 0);
  }      
}




int do_mkdir (const char * path, mode_t mode){

	char * delim = "/";
	int found =0;
	int i=0;
	char *temp, *temp2;

	struct file_entry * new_entry;
	struct file_entry * traverse = root;
	struct inode * new_inode;

	//maybe not needed
	//if (path[0] = '/'){
		temp = strdup(path + 1);
		while (1){
			
			temp2 = strsep(&temp, delim);

			//work
			if (temp != NULL){


				for (i= 0; i < traverse -> no_files; i++){
					found = 0;
					if (strcmp (traverse -> all_files[i][0].file_name, temp) == 0 && inode_entries[traverse -> all_files[i][0].inode_num].file_type == DIRECTORY){//also check if it is a directory, inode nmber is required
						found = 1;
						traverse = (traverse -> all_files[i]);					
					}
				}
				
				if (found == 0){
					
					//path not found error
					return -1;
				}
			}
			
		}
  	
    create_inode (new_inode, DIRECTORY);
    create_file_entry (new_entry, path, DIRECTORY);

    if (traverse -> no_files == traverse -> max_file_count)
      resize_all_files (traverse);
    
    traverse -> all_files[traverse -> no_files]  = new_entry;
    traverse -> no_files++;

    return 0;




}


static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	fflush(stdout);
	st->st_uid = 0; //getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = 0; //getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
  printf("ret_inode %d\n", ret_inode(path));
  if(ret_inode(path) == -1)
    return -ENOENT;
  else{
  struct inode inod = inode_entries[ret_inode(path)];
  memset(st, 0, sizeof(struct stat));
  if(sizeof(inod) == 0)
    

	if( strcmp( path, "/" ) == 0)
	{
    printf("root\n\n\n");
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}
  
  else if(inod.file_type == DIRECTORY){
    st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
  }
  
	else if(inod.file_type == REGULAR)
	{
    printf("not root\n\n\n");
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;

	}
  return 0;
  }
	
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
  printf("\nReading the directory  %s\n", path);

  /*
  if(strcmp(path, "/") != 0)
    return -ENOENT;
  */
  int i = 0;
  filler(buffer, ".", NULL, 0, 0);
  filler(buffer, "..", NULL, 0, 0);
  //filler(buffer, "test", NULL, 0, 0);
  
  struct file_entry *fentry;
  struct inode inod;
  
    if(ret_inode(path) != -1){
      inod = inode_entries[ret_inode(path)];
      if(inod.file_type == DIRECTORY){
        while(fentry->all_files[i] != NULL){
          filler(buffer, fentry->all_files[i]->file_name, NULL, 0, 0);
        }
        return 0;
      }

    }
  return -1;
  
    }




static int do_read (const char *path, char *buffer, size_t size, off_t off, struct fuse_file_info *fi){
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
          struct inode inode_entry = inode_entries[file_entries[i]->inode_num];
          
          char* content = malloc((inode_entry.blks_in_use * BLK_SIZE)+1);

          int offset = 0, i;

          for(i=0; i<inode_entry.blks_in_use; i++){
            memcpy(&content[offset], &inode_entry.block_addrs[i], BLK_SIZE); //not sure if it works
            offset+=BLK_SIZE;
          }

          memcpy(buffer, content+off, size);
          free(content);
          return strlen(content)-off;

        
      }
    }
    return -ENOENT;
    }
}


static int do_open(const char *path, struct fuse_file_info *fi){

  int i;
      
  if((fi->flags & O_ACCMODE) == O_RDONLY){
    return 0;
  }
  if((fi->flags & O_ACCMODE) == O_CREAT){
    if(do_mknod(path, S_IFREG, 0) == 0)
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
    printf("Writing to a file");
    
    int size = sizeof(file_entries)/sizeof(file_entries[0]);
    int i;

    for(i=0; i<size; i++){
      if(strcmp(path, file_entries[i]->file_name) == 0){
        struct inode inode_entry = inode_entries[file_entries[i]->inode_num];
        int offset = 0, j;
        
        for(j=0; j<inode_entry.blks_in_use; j++){
            memcpy( &inode_entry.block_addrs[j], &buffer[offset], BLK_SIZE); //not sure if it works
            offset+=BLK_SIZE;
          }
      return strlen(buffer) - off;
  }
  return -ENOENT;
}
  }
  return 0;
}


static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
    .mknod = do_mknod,
    .mkdir = do_mkdir
};

int main(int argc, char *argv[])
{
  init_storage();
	return fuse_main( argc, argv, &operations, NULL);
}