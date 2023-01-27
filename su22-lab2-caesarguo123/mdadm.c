//CMPSC 311 SP22
//LAB 2

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"


/*

    tempa = start_addr&0x00;
    tempb = (read_len&0x00) <<1;
    tempc = (*read_buf&0x00) <<24;
    retval = tempa|tempb|tempc;
    return retval;
*/

 uint32_t jbod( uint32_t diskID, uint32_t blockID, uint32_t command, uint32_t reserved){ // make a jbod to put all the op.
    uint32_t tempa, tempb,tempc,tempd, retval= 0;  //  this is by following class video, use temp place to left forward bits.
    tempa = (diskID) << 28;  // left forward 28 bits following table 1.
    tempb = (blockID) << 20;  // left forward 20 bits following table 1.
    tempc = (command) <<14;
    tempd = (reserved);  // no need to left forward, since this is 0-13 bits.
    retval = tempa|tempb|tempc|tempd;  // use retval to put this together as a 32 bits jbod.
    return retval;
  }


int mounted = 0;  // create a "mounted" to check if mount or not, and it is initially 0 .

int mdadm_mount(void) {  

  if (mounted == 0){  // if check mount find not mount.
    jbod_operation(JBOD_MOUNT, NULL);  // call mount op.
    mounted =1;  // mount op will set mount to 1, which is mounted now.
    return 1; // return success.
  }
  return -1;   //else, it fail to call mount.

}

int mdadm_unmount(void) {

  if (mounted == 1){  // if check mount and find already mounted.
    jbod_operation(JBOD_UNMOUNT, NULL); // call ummount op.
    mounted = 0;  // set mount back to 0, which is not mount.
    return 1;   // return
  }
  return -1;  // else, fail to unmount.
}

int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf) {  
 
  if (mounted ==0){  //  if not mount, fail read.
    return -1;
  }
  if (start_addr + read_len >1048576){   // this is a 16*65536 hard drive, max will be 1048576. anything over that will fail to read.
    return -1;
  }
  if (read_len >1024){  // read length bigger than 1024 will fail.
    return -1;
  }
  if (read_buf == NULL && read_len != 0){ // read_buf can not be null if read_len is not 0.
    return -1;
  }
  if (read_buf == NULL && read_len ==0){  // read_buf be null, and read_len be 0 can be pass.
    return 0;
  }
  
    //jbod_operation(JBOD_SEEK_TO_DISK, NULL);
    //jbod_operation(JBOD_SEEK_TO_BLOCK, NULL);
    //memcpy(&read_buf, &read_len, 16);
 
  int CurrentDiskID, CurrentBlockID;  // set thsoe two to track number of disk and block.
  int block_spaceused, block_spaceleft;  // use to know position inside a block.
  //int tempstart, tempend;
  //uint8_t *tempblock;
  CurrentDiskID = start_addr/ 65536;  // gives number of disk
  CurrentBlockID = (start_addr % 65536)/ 256; // this is same as (currentDiskID - (int)currentDiskID)*256
  block_spaceused = (start_addr %256); // how many bytes have already used in this block.
  block_spaceleft = 256 - (start_addr%256); // how many bytes left in this block.
  
  
  //printf ("%d.", block_spaceused);
  // printf ("%d.", CurrentBlockID);   //use to track all of those data, and help debug.
   // printf ("%d.", CurrentDiskID);
    // printf ("%d.", block_spaceleft);

/* 


      if (block_spaceused==0 && block_spaceleft ==256 ){
        memcpy( read_buf, &temp[block_spaceused], read_len);
        start_addr += read_len;
      }
*/

     
     
    uint8_t temp[256];   //use this temp place to hold address.
  
    int needread= read_len;  // since we cant change read_len, and need to modified read_len to read cross block inside loop. we make a clone read_len.
    int readed = 0; // a temp place to hold all block_spaceused add together cross all disk/block, use to track how many bytes already readed inside loop.

    while (needread >=1 ){ // use a while loop keep looping, stop till no more read_len need to read.

       uint32_t seekdisk =jbod(CurrentDiskID,0, JBOD_SEEK_TO_DISK, 0);  // make a seekdisk op, follow by our jbod architecture, ignored CurrentBlockID since seek disk.
        jbod_operation(seekdisk, NULL);
        
 
        uint32_t seekblock = jbod(0, CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0);// make a seekblock op, follow by our jbod architecture, ignored currentDiskID since seek Block.
        jbod_operation(seekblock, NULL);

        uint32_t readblock = jbod(0, 0, JBOD_READ_BLOCK, 0); // read op, all other fields are ignored.
        jbod_operation(readblock, temp);
      

     /* if (block_spaceused==0 && block_spaceleft ==256 ){
        memcpy( read_buf, &temp[block_spaceused], read_len);
        start_addr += read_len;
      }*/
        

       
      if ( block_spaceleft < needread) {  // under this while loop, if we dont have enough space under this block to read.


        CurrentDiskID = start_addr/ 65536;
        CurrentBlockID = (start_addr % 65536)/ 256;  
        block_spaceused = (start_addr %256);
        block_spaceleft = 256 - (start_addr%256);

        uint32_t seekdisk =jbod(CurrentDiskID,0, JBOD_SEEK_TO_DISK, 0);
        jbod_operation(seekdisk, NULL);
 
        uint32_t seekblock = jbod(0, CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0);  //thsoe are make sure after each loop, all numbers are chanege , since start_addr will be change after each read.
        jbod_operation(seekblock, NULL);

        uint32_t readblock = jbod(0, 0, JBOD_READ_BLOCK, 0);
        jbod_operation(readblock, temp);


        memcpy(&read_buf[readed], &temp[block_spaceused],block_spaceleft);  // memcpy the block_spaceleft as length, copy block part to buf.
        start_addr += block_spaceleft;  // start_addr will increase since we read block_spaceleft length of bytes . this will help to track DIskID, BlockID, blockspaceused in next function of loop.
        readed += block_spaceleft; // now the total bytes we readed is increase as length of block_space left, since we dont have enough space to read all at once. this data will go to next function in looping.
        //block_spaceused += block_spaceleft;  //this will always full, always equal to 256.
        needread -= block_spaceleft;  // still have some needread after this read, we need update needread since we will go to next block.
        //block_spaceleft = 256; //256 as next block have this much left.
       
       /*
       printf("fun1.");
       printf("%d.", read_len);
       printf("%d.", readed);
       printf("%d.", start_addr);
       printf("%d.", needread);
      */
     
       

      }
      
      
      //if (needread <= block_spaceleft ){   // we have enough space inside this block to read all we need read as once.
      else{
        CurrentDiskID = start_addr/ 65536;
        CurrentBlockID = (start_addr % 65536)/ 256;
        block_spaceused = (start_addr %256);
        block_spaceleft = 256 - (start_addr%256);
        
        uint32_t seekdisk =jbod(CurrentDiskID,0, JBOD_SEEK_TO_DISK, 0);
        jbod_operation(seekdisk, NULL);
        uint32_t seekblock = jbod(0, CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0);  //thsoe are make sure after each loop, all numbers are chaneged , since start_addr will be change after each read.
        jbod_operation(seekblock, NULL);
        uint32_t readblock = jbod(0, 0, JBOD_READ_BLOCK, 0);
        jbod_operation(readblock, temp);



        memcpy(&read_buf[readed], &temp[block_spaceused] ,needread);  // now that space_left is bigger than what we neeed read, just use needread as length. And copy memrory from this block to buf.
        needread -= needread ; // this will always be 0. if we can get to this part, no more bytes need to read. while loop is end.
    
      /*
      printf("fun2.");
       printf("%d.", read_len);
       printf("%d.", readed);
       printf("%d.", start_addr);
       printf("%d.", needread);
       */
      }

    } 
   
return read_len;
   
}    


