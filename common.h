extern int BLOCKSIZE;
int CURR_DBLOCK_IN_NEW;
typedef struct {
  int blocksize; /* size of blocks in bytes */
  int inode_offset; /* offset of inode region in blocks */
  int data_offset; /* data region offset in blocks */
  int swap_offset; /* swap region offset in blocks */
  int free_inode; /* head of free inode list */
  int free_block; /* head of free block list */
}st_superblock;

#define BOOT_SUP_AREA 1024
#define N_DBLOCKS 10 
#define N_IBLOCKS 4  
typedef struct {  
  int next_inode; /* list for free inodes */  
  int protect;        /*  protection field */ 
  int nlink;  /* Number of links to this file */ 
  int size;  /* Number of bytes in file */   
  int uid;   /* Owner's user ID */  
  int gid;   /* Owner's group ID */  
  int ctime;  /* Time field */  
  int mtime;  /* Time field */  
  int atime;  /* Time field */  
  int dblocks[N_DBLOCKS];   /* Pointers to data blocks */  
  int iblocks[N_IBLOCKS];   /* Pointers to indirect blocks */  
  int i2block;     /* Pointer to doubly indirect block */  
  int i3block;     /* Pointer to triply indirect block */  
}st_inode;
int defragmenter(   unsigned char * buffer,
                    unsigned char * newBuffer,
                    st_superblock *pSB,
                    st_inode *inode);
extern int copyDataBlock();