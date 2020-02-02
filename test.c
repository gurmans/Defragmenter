#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int BLOCKSIZE;
int getDiskSize(FILE * f);
int calculateFileBlocks(int size);

int main(int argc,char* argv[]){
    FILE * f;
    FILE * fNew;
    unsigned char * buffer;
    unsigned char * newBuffer;
    size_t bytes;
    if(argc < 2 || argv[1] == NULL || argv[1] == "")
    {
        printf("Please enter a valid name for the disk image");
        return -1;
    }
    // printf("Number of args: %d\n",argc);
    f = fopen(argv[1],"rb");
    fNew = fopen("disk_defrag","wb");
    int diskSize = getDiskSize((FILE *)f);
    // copyDataBlock();
    buffer = malloc(diskSize);
    newBuffer = malloc(diskSize);
    bytes =  fread(buffer,BOOT_SUP_AREA,1,f);
    // fread(newBuffer,diskSize,1,fNew);

    st_superblock *pSB = (st_superblock*) &(buffer[512]);
    BLOCKSIZE = pSB->blocksize;
    // printf("free_block%d\n",pSB->free_block);

    // printf("blocksize%d\n",pSB->blocksize);
    int readindex = 0;
    for(;BOOT_SUP_AREA+readindex<diskSize;readindex+=BLOCKSIZE)
        bytes =  fread(&buffer[BOOT_SUP_AREA+readindex],BLOCKSIZE,1,f);

    st_inode *inode = (st_inode*) &(buffer[BOOT_SUP_AREA + pSB->inode_offset*BLOCKSIZE]);

    CURR_DBLOCK_IN_NEW = 0;
    defragmenter(buffer,newBuffer,pSB,inode);
    // printf("Number of Blocks : %d", diskSize/BLOCKSIZE-pSB->data_offset);

    pSB->free_block = CURR_DBLOCK_IN_NEW;
    int totalDataBlocks =  (diskSize/BLOCKSIZE)-pSB->data_offset;
    int *intBuffer = newBuffer;
    for(;pSB->data_offset+CURR_DBLOCK_IN_NEW<pSB->swap_offset;CURR_DBLOCK_IN_NEW++)
    {
        intBuffer[(BOOT_SUP_AREA+BLOCKSIZE*(pSB->data_offset+CURR_DBLOCK_IN_NEW))/sizeof(int)] = CURR_DBLOCK_IN_NEW+1;
    }
    CURR_DBLOCK_IN_NEW--;
    intBuffer[(BOOT_SUP_AREA+BLOCKSIZE*(pSB->data_offset+CURR_DBLOCK_IN_NEW))/sizeof(int)] = -1;
    // TODO: update superblock with new pointer to free data blocks
    memcpy(newBuffer,buffer,BOOT_SUP_AREA);//copying boot and superblock area
    memcpy(&newBuffer[BOOT_SUP_AREA + pSB->inode_offset*BLOCKSIZE],
                &buffer[BOOT_SUP_AREA + pSB->inode_offset*BLOCKSIZE],
                BLOCKSIZE*(pSB->data_offset-pSB->inode_offset));

    char * swapBuffer = &buffer[BOOT_SUP_AREA + (pSB->swap_offset)*BLOCKSIZE];
    char * newSwapBuffer = &newBuffer[BOOT_SUP_AREA + (pSB->swap_offset)*BLOCKSIZE];

    int index = 0;
    for(;(int)&buffer[diskSize]>(int)&swapBuffer[index*4];index+=BLOCKSIZE/4)
    {
        char * target = &newSwapBuffer[index*4];
        char * source = &swapBuffer[index*4];
        memcpy(target,source,BLOCKSIZE);
    }

    // printf("free_block%d\n",pSB->free_block);
    // memcpy(newSwapBuffer,
    //             swapBuffer,
    //             25600);// diskSize - ((pSB->swap_offset)*BLOCKSIZE));
    // memcpy(&newBuffer[5242880],
    //             &buffer[5242880],200);
    //             // diskSize - ((pSB->swap_offset)*BLOCKSIZE));
    fwrite(newBuffer, diskSize, 1, fNew);
    // printf("loop%d\n",index);
    fclose(fNew);
    fclose(f);
}

int getDiskSize(FILE * f)
{
    fseek(f, 0, SEEK_END); // seek to end of file
    int diskSize = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET); // seek back to beginning of file
    diskSize+=diskSize%4==0?0:4-diskSize%4;
    return diskSize;
}

int calculateFileBlocks(int size)
{
    if(size%BLOCKSIZE == 0)
        return size/BLOCKSIZE;
    return size/BLOCKSIZE + 1;
}
