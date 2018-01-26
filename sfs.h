
#define MAX_LEN 32
#define BLKS_PER_INODE 5
#define BLK_SIZE 1024

//inode_num : keep positive numbers for files/directories
//            keep -1 for end of dir_entries
//            0 - '.' (default directory entry)
//            1 - '..' (default directore entry)
struct dir_entry{
    int inode_num;
    char file_name[MAX_LEN];
};


struct dir_entry **dir_entries; //static allocation: an option.


enum FILE_TYPE {
        UNUSED=0,
        REGULAR=1,
        DIRECTORY
};



// Basic inode definition
struct inode{
    enum FILE_TYPE file_type;
    char owner_id[32];
    int access_permission;
    int last_accessed; //in epochs maybe
    int last_modified; //in epochs maybe
    int inode_last_mod; //in epochs maybe
    int file_size;
    //int link_count;
    int blks_in_use;
    int block_addrs[BLKS_PER_INODE];
    //int first_ind_blk_addr;  address of indirect blocks
    //int second_ind_blk_addr;
};

struct inode **inode_entries;  //static allocation: an option.