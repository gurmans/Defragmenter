#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int copyDataBlock(st_inode *inode,
                    int  dataStartupAddr,
                    int  dataStartupNewAddr,
                    unsigned char * buffer,
                    unsigned char * newBuffer,
                    int p,int i);
int copyL1Block(st_inode *ith_inode,int blockIndex, int *nFileBlocks, 
        st_superblock * pSB, unsigned char *buffer, unsigned char *newBuffer);


int defragmenter(   unsigned char * buffer,
                    unsigned char * newBuffer,
                    st_superblock * pSB,
                    st_inode * inode)
{
    typedef struct{
        unsigned char byte[BLOCKSIZE];
    } st_dataPointer;
    st_dataPointer * dataStartup = &buffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];
    st_dataPointer * dataStartupNew = &newBuffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];
    int i = 0;
    for(;(int)dataStartup-(int)&inode[i]>=sizeof(st_inode);i++)
    {
        if(inode[i].nlink>0)
        {
            int nFileBlocks = calculateFileBlocks(inode[i].size);
            // printf("#fileblocks %d\n",inode[i].size);
            int p = 0;
            for(;nFileBlocks>0 && p<N_DBLOCKS;nFileBlocks--,p++)
            {
                int sourceOffset = inode[i].dblocks[p];
                char * dbAddr = &dataStartup[sourceOffset];
                char * newDBAddr = &dataStartupNew[CURR_DBLOCK_IN_NEW];
                            memcpy(newDBAddr,dbAddr,BLOCKSIZE);
                inode[i].dblocks[p] = CURR_DBLOCK_IN_NEW;
                CURR_DBLOCK_IN_NEW++;
            }
            //iDBlocks
            for(;nFileBlocks>0 && p<N_DBLOCKS+N_IBLOCKS;p++)
            {
                copyL1Block(&inode[i], p - N_DBLOCKS, &nFileBlocks, pSB, buffer, newBuffer);
            }
            //i2DBlock
            // printf("File Blocks Left after l1:%d\n", nFileBlocks);
            if(nFileBlocks>0)
            copyL2Block(&inode[i].i2block, &nFileBlocks, pSB, buffer, newBuffer);
            // printf("File Blocks Left after l2:%d\n", nFileBlocks);
            if(nFileBlocks>0)
            copyL3Block(&inode[i].i3block, &nFileBlocks, pSB, buffer, newBuffer);
            // printf("File Blocks Left after l3:%d\n", nFileBlocks);

        }
        // printf("%2d ",i);
    }

    // indirect pointer block - indirect data blocks
    // TODO: set free datablock list
}

int copyL1Block(st_inode *ith_inode,int blockIndex, int *nFileBlocks, 
        st_superblock * pSB, unsigned char *buffer, unsigned char *newBuffer){
    typedef struct{
        unsigned char byte[BLOCKSIZE];
    } st_dataPointer;
    st_dataPointer * dataStartup = &buffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];
    st_dataPointer * dataStartupNew = &newBuffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];
    unsigned int *l1PointersBlock = &dataStartup[ith_inode[0].iblocks[blockIndex]];
    unsigned int *l1PointersBlockNew = &dataStartupNew[CURR_DBLOCK_IN_NEW];
    // memcpy(l1PointersBlockNew,l1PointersBlock,BLOCKSIZE);
    int l1Offset = CURR_DBLOCK_IN_NEW;
    int n_l1Pointers = (int)floor((double)BLOCKSIZE/sizeof(int));
    int ind = 0;
    CURR_DBLOCK_IN_NEW++;
    for(;ind<n_l1Pointers && *nFileBlocks>0;*nFileBlocks-=1,ind++)
    {
        l1PointersBlockNew[ind] = CURR_DBLOCK_IN_NEW;
        char * sourceAddress = &dataStartup[l1PointersBlock[ind]];
        char * targetAddress = &dataStartupNew[l1PointersBlockNew[ind]];
        memcpy(targetAddress,sourceAddress,BLOCKSIZE);
        CURR_DBLOCK_IN_NEW++;
    }
// printf("AAAAAAABB%d\n", &dataStartup[CURR_DBLOCK_IN_NEW]);
    for(;ind<n_l1Pointers;ind++)
    {
        l1PointersBlockNew[ind] = -1;
    }
    ith_inode[0].iblocks[blockIndex] = l1Offset;

}

int copyL3Block(int *pointer, int *nFileBlocks, 
        st_superblock * pSB, unsigned char *buffer, unsigned char *newBuffer){
    typedef struct{
        unsigned char byte[BLOCKSIZE];
    } st_dataPointer;
    st_dataPointer * dataStartup = &buffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];
    st_dataPointer * dataStartupNew = &newBuffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];

    unsigned int *l3PointersBlock = &dataStartup[*pointer];
    unsigned int *l3PointersBlockNew = &dataStartupNew[CURR_DBLOCK_IN_NEW];
    // memcpy(l2PointersBlockNew,l2PointersBlock,BLOCKSIZE);
    int l3OffsetTmp = CURR_DBLOCK_IN_NEW;
    int n_l3Pointers = (int)floor((double)BLOCKSIZE/sizeof(int));
    int ind = 0;
    CURR_DBLOCK_IN_NEW++;
    for(;ind<n_l3Pointers && *nFileBlocks>0;ind++)
    {
        l3PointersBlockNew[ind] = CURR_DBLOCK_IN_NEW;
        int sourceL2DBAddress = &dataStartup[l3PointersBlock[ind]];
        int targetL2DBAddress = &dataStartupNew[l3PointersBlockNew[ind]];
        copyL2Block(&l3PointersBlock[ind],nFileBlocks,pSB,buffer,newBuffer);
    }
    for(;ind<n_l3Pointers;ind++)
    {
        l3PointersBlockNew[ind] = -1;
    }
    *pointer = l3OffsetTmp;

}

int copyL2Block(int *pointer, int *nFileBlocks, 
        st_superblock * pSB, unsigned char *buffer, unsigned char *newBuffer){
    typedef struct{
        unsigned char byte[BLOCKSIZE];
    } st_dataPointer;
    st_dataPointer * dataStartup = &buffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];
    st_dataPointer * dataStartupNew = &newBuffer[BOOT_SUP_AREA+BLOCKSIZE*pSB->data_offset];
    unsigned int *l2PointersBlock = &dataStartup[*pointer];
    unsigned int *l2PointersBlockNew = &dataStartupNew[CURR_DBLOCK_IN_NEW];
    // memcpy(l2PointersBlockNew,l2PointersBlock,BLOCKSIZE);
    int l2OffsetTmp = CURR_DBLOCK_IN_NEW;
    int n_l2Pointers = (int)floor((double)BLOCKSIZE/sizeof(int));
    int ind = 0;
    CURR_DBLOCK_IN_NEW++;
    for(;ind<n_l2Pointers && *nFileBlocks>0;ind++)
    {
        l2PointersBlockNew[ind] = CURR_DBLOCK_IN_NEW;
        int sourceL1DBAddress = (int)&dataStartup[l2PointersBlock[ind]];
        int targetL1DBAddress = (int)&dataStartupNew[l2PointersBlockNew[ind]];
        // memcpy(targetL1DBAddress,sourceL1DBAddress,BLOCKSIZE);
        //add l1
        unsigned int * l1PointersBlock = &dataStartup[l2PointersBlock[ind]];
        unsigned int * l1PointersBlockNew = &dataStartupNew[l2PointersBlockNew[ind]];
        CURR_DBLOCK_IN_NEW++;
        int l1Offset = CURR_DBLOCK_IN_NEW;
        int n_l1Pointers = (int)floor((double)BLOCKSIZE/sizeof(int));
        int ind9 = 0;
        // CURR_DBLOCK_IN_NEW++;
        for(;ind9<n_l1Pointers && *nFileBlocks>0;*nFileBlocks-=1,ind9++)
        {
            l1PointersBlockNew[ind9] = CURR_DBLOCK_IN_NEW;
            char * sourceAddressD = &dataStartup[((int)l1PointersBlock[ind9])];
            char * targetAddressD = &dataStartupNew[((int)l1PointersBlockNew[ind9])];
            memcpy(targetAddressD,sourceAddressD,BLOCKSIZE);
            // printf("AAAAAAACCC%d\n", &dataStartup[((int)l1PointersBlock[ind9]) * BLOCKSIZE]);
            CURR_DBLOCK_IN_NEW++;
        }
        for(;ind9<n_l1Pointers;ind9++)
        {
            l1PointersBlockNew[ind9] = -1;
        }

        // l2PointersBlock[ind] = l1Offset;
        // ith_inode[0].iblocks[blockIndex] = l1Offset;

    }
    for(;ind<n_l2Pointers;ind++)
    {
        l2PointersBlockNew[ind] = -1;
    }
    *pointer = l2OffsetTmp;
}

int copyDataBlock
                (   st_inode *inode,
                    int  dataStartupAddr,
                    int  dataStartupNewAddr,
                    unsigned char * buffer,
                    unsigned char * newBuffer,
                    int p,int i)
    {
    typedef struct{
        unsigned char byte[BLOCKSIZE];
    } st_dataPointer;
    // st_inode *inode = inodeAdr;
    st_dataPointer * dataStartup =  dataStartupAddr;
    st_dataPointer * dataStartupNew =  dataStartupNewAddr;
// printf("AAAAAAA%d\n", inode[1].nlink);
    int sourceOffset = inode[i].dblocks[p];
    char * dbAddr = &dataStartup[sourceOffset];
    char * newDBAddr = &dataStartupNew[CURR_DBLOCK_IN_NEW];
    memcpy(newDBAddr,dbAddr,BLOCKSIZE);
    inode[i].dblocks[p] = CURR_DBLOCK_IN_NEW;
    CURR_DBLOCK_IN_NEW++;

}