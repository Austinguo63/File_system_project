#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"

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
        CurrentDiskID = start_addr/ 65536;  // gives number of disk
        CurrentBlockID = (start_addr % 65536)/ 256; // this is same as (currentDiskID - (int)currentDiskID)*256
        block_spaceused = (start_addr %256); // how many bytes have already used in this block.
        block_spaceleft = 256 - (start_addr%256); // how many bytes left in this block.
  
      
        

        
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


        memcpy(read_buf+readed, temp+block_spaceused,block_spaceleft);  // memcpy the block_spaceleft as length, copy block part to buf.
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
      
      else{   // we have enough space inside this block to read all we need read as once.

        CurrentDiskID = start_addr/ 65536;
        CurrentBlockID = (start_addr % 65536)/ 256;
        block_spaceused = (start_addr %256);
        block_spaceleft = 256 - (start_addr%256);
        //int i = 256;

        uint32_t seekdisk =jbod(CurrentDiskID,0, JBOD_SEEK_TO_DISK, 0);
        jbod_operation(seekdisk, NULL);
        uint32_t seekblock = jbod(0, CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0);  //thsoe are make sure after each loop, all numbers are chaneged , since start_addr will be change after each read.
        jbod_operation(seekblock, NULL);
        uint32_t readblock = jbod(0, 0, JBOD_READ_BLOCK, 0);
        jbod_operation(readblock, temp);



        memcpy(read_buf+readed, temp+block_spaceused ,needread);  // now that space_left is bigger than what we neeed read, just use needread as length. And copy memrory from this block to buf.
        readed = readed + 256;
        needread = 0 ; // this will always be 0. if we can get to this part, no more bytes need to read. while loop is end.
    
      
      
      /*printf("fun2.");
       printf("%d.", read_len);
       printf("%d.", readed);
       printf("%d.", start_addr);
       printf("%d.", needread);
       */


    }
  }
   
return read_len;
}






int mdadm_write(uint32_t start_addr, uint32_t write_len, const uint8_t *write_buf) {
 // printf("***\nstaraddr:%d.length:%d.",start_addr,write_len  );
  if (mounted ==0 || start_addr+ write_len >1048576 || write_len >1024 ||(write_buf == NULL && write_len != 0)){  //follow all the requirement in tester, orders are followed by tester as well.
    return -1;
  }
  if (write_buf == NULL && write_len ==0){ // allow to have blank write, o length and nell pointer.
    return 0;
  }
  


  int writed = 0;  // use this to track how many are writed already.
  int needwrite = write_len; // clone a write_len to modifide in while loop.
  uint32_t seekdisk, seekblock, writeblock, readblock; // create op.

  int CurrentDiskID, CurrentBlockID;  // set thsoe two to track number of disk and block.
  int block_spaceused, block_spaceleft;  // use to know position inside a block.
  CurrentDiskID = start_addr/ 65536;  // gives number of disk
  CurrentBlockID = (start_addr % 65536)/ 256; // this is same as (currentDiskID - (int)currentDiskID)*256
  block_spaceused = (start_addr %256); // how many bytes have already used in this block.
  block_spaceleft = 256 - (start_addr%256); // how many bytes left in this block.
  uint8_t local[256];



  while (needwrite >=1){ // if needread is now 0, means still have byte need to write, keep going. only stop if all bytes are done. 
      
      CurrentDiskID = start_addr/ 65536;  // gives number of disk
      CurrentBlockID = (start_addr % 65536)/ 256; // this is same as (currentDiskID - (int)currentDiskID)*256
      block_spaceused = (start_addr %256); // how many bytes have already used in this block.
      block_spaceleft = 256 - (start_addr%256);


    if (needwrite > block_spaceleft){ // if we have enough space left in block that can write this much of byte.

      CurrentDiskID = start_addr/ 65536;  // gives number of disk
      CurrentBlockID = (start_addr % 65536)/ 256; // this is same as (currentDiskID - (int)currentDiskID)*256
      block_spaceleft = 256 - (start_addr%256); // how many bytes left in this block.
      block_spaceused = (start_addr %256); // how many bytes have already used in this block.

    

      seekdisk = jbod (CurrentDiskID , 0, JBOD_SEEK_TO_DISK, 0);  // seek disk and block to get current location inside this if before read.
      jbod_operation( seekdisk, NULL);  
      seekblock = jbod(0,CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0); // get the block id in current disk.
      jbod_operation(seekblock, NULL );
      readblock = jbod(CurrentDiskID, CurrentBlockID, JBOD_READ_BLOCK, 0); // need read cuerrent block to know what to write.
      jbod_operation(readblock, local);
      seekdisk = jbod (CurrentDiskID , 0, JBOD_SEEK_TO_DISK, 0);  // seek disk and block to get current location inside this if before read.
      jbod_operation( seekdisk, NULL);   
      seekblock = jbod(0,CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0); // get the block id in current disk.
      jbod_operation(seekblock, NULL );


      //printf("fun1.needwrite:%d.spaceinblock%d.",needwrite,block_spaceleft);
      //printf("disk:%d.Block:%d\n",CurrentDiskID,CurrentBlockID);
      memcpy (local+block_spaceused, write_buf+writed, block_spaceleft);
      //memcpy (local, write_buf+block_spaceused, block_spaceleft);
      start_addr = start_addr + block_spaceleft; // startaddr will update because new bytes are in.
      writed = writed + block_spaceleft; // writed will update as well cause some length is in
      needwrite -=block_spaceleft; // write_len will - bytes already writed.
  
      writeblock = jbod(CurrentDiskID, CurrentBlockID, JBOD_WRITE_BLOCK, 0); // now call op after we memcpy to the local memory.
      jbod_operation(writeblock, local); // pointer will be local since this is to local memory.
  
      

    }
    

    else {
      CurrentDiskID = start_addr/ 65536;  // gives number of disk
      CurrentBlockID = (start_addr % 65536)/ 256; // this is same as (currentDiskID - (int)currentDiskID)*256
      block_spaceused = (start_addr %256); // how many bytes have already used in this block.
      block_spaceleft = 256 - (start_addr%256); // how many bytes left in this block.

    

      seekblock = jbod(0,CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0);  // use seekblock to see what block we are currentlly in.
      jbod_operation(seekblock, NULL );
      seekdisk = jbod (CurrentDiskID , 0, JBOD_SEEK_TO_DISK, 0);// check what disk we are currentlly in.
      jbod_operation( seekdisk, NULL);
      readblock = jbod(CurrentDiskID, CurrentBlockID, JBOD_READ_BLOCK, 0); // read this block inorder to know what contant to write.
      jbod_operation(readblock, local);
      seekdisk = jbod (CurrentDiskID , 0, JBOD_SEEK_TO_DISK, 0);  // seek disk and block to get current location inside this if before read.
      jbod_operation( seekdisk, NULL);  
      seekblock = jbod(0,CurrentBlockID, JBOD_SEEK_TO_BLOCK, 0); // get the block id in current disk.
      jbod_operation(seekblock, NULL );
  
    
      //memcpy(&local, write_buf +block_spaceused, needwrite);
      //printf("staraddr:%d.length:%d.",start_addr,write_len  );
      //printf("fun2.needwrite:%d.spaceinblock%d.",needwrite,block_spaceleft);
      //printf("disk:%d.Block:%d\n",CurrentDiskID,CurrentBlockID);
      memcpy (local+block_spaceused, write_buf+writed, needwrite);
      writed += needwrite; //update writed with number of bytes writed.
      start_addr = start_addr + needwrite; //update start_addr + the number that we writed already.
      needwrite =0; // this will always be 0 since we have enough space inside block, so this one block will fit all bytes we want to write.

      writeblock = jbod(CurrentDiskID, CurrentBlockID, JBOD_WRITE_BLOCK, 0); // call write op when we finsh memcpy bytes to local memory.
      jbod_operation(writeblock, local);
      

      }
    
      
  


  }




 
  	return write_len;
}
