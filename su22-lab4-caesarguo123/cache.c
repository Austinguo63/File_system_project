#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
//static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;

int cacheEnabled = 0;
int fifoIndex = 0;
int created =0;  //use this to check if a cache is created or not. set to 0 as not created.

int cache_create(int num_entries) {
  int temp;
  if (created != 0){  //if cache is already created, return fail.
    return -1;
  }
  if (num_entries > 4096 || num_entries < 2 ){ //rules about num_entries.
    return -1;
  }
  else {
    int space_need;  // use this to check what is max space neeeded.
    created =1; // now make created as 1, since we success created a cache.
    cache_size = num_entries; // follow guide
    temp = sizeof(cache_entry_t); //get the size of this uint8 char.
    space_need = cache_size * temp; //now this give us max space we need for memory allocate.
    cache = malloc(space_need); // auto allocate this much of memory.
    int x;
    for (x =0; x< cache_size; x++){ //use a for loop to set each cache loop.
      cache[x].valid = false; // set false as default 
      cache[x]. disk_num = 0; // 0 as default disk and block number.
      cache[x]. block_num = 0;
    }



    //printf ("%d.", max_capacity);
    //printf ("%d.", cache_size);
   // printf ("%d.", temp);
    




  }

  return 1;
}

int cache_destroy(void) { 
  if (created != 1){  // if created is already set as 0 , no need to destroy, return fail.
    return -1;
  }
  else {  //if after check that we have created =1.
    created = 0; // destroy it.
    free(cache); // free cache as guided.
    cache_size = 0; //set cache size to 0.
    cache = NULL;


  }
  
  return 1;
  
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  
  if (cache == NULL){  //empty cache sould fail
    return -1;
  }
  if ( created == 0){ //fail if not creat cache.
    return -1; 
  }
  if (disk_num <0 || block_num<0){ // disk and block can not be - numbers.
    return -1;
  }
  if (cache_size <1){ //cache size at least 1.
    return -1;
  }
  if (disk_num >= 16 || block_num >= 256){ // no bigger than max disk block size.
    return -1;
  }
  if (buf == NULL){ //null buf should fail
    return -1; 
  }
  
 
  else{
    int x; 
    uint8_t *copyblock; // use to copy cache.
    for (x=0; x<cache_size; x++){
      if(cache[x].valid == false){
        return -1;
      }
      if (cache[x].valid == true && disk_num == cache[x].disk_num && block_num == cache[x].block_num){
        copyblock = cache[x].block;
        if (copyblock == NULL){
          return -1;
        }
        else{
          int size = JBOD_BLOCK_SIZE;
          memcpy (buf, copyblock, size);
          num_hits = num_hits +1;
          return 1;
        }

      }
      return 1;
    }


    /*
    num_queries = num_queries +1 ; 
    for (x =0; x < cache_size; x++){
      if (disk_num == cache[x].disk_num && block_num == cache[x].block_num){ // if our cache[x] already have same disk block number with what given.
        copyblock = cache[x].block; // if block disk number match, we pointer point to this block.
        if(cache[x].valid == false || copyblock == NULL){ //if this is a empty cache, by having null block or not valid.
          return -1; //fail
        }
        else{
          int size =JBOD_BLOCK_SIZE; //if not empty, we set copy size as block max size.
          memcpy (buf, copyblock,size); // mem copy 256 from exist match block to buf.
          num_hits = num_hits +1; // since this is a match, add hit number by 1.
          return 1;
          
        }
        
      } 
      return 1;
  }
  */
  }
  return -1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  int x;
  uint8_t * copyblock;
  for (x=0; x< cache_size; x++){  // for update, x <cache_size
    if (cache[x].valid == true){ // if we have a valid non-empty cache.
      if (disk_num == cache[x].disk_num && block_num == cache[x].block_num){ // and this is a exist cache
        copyblock = cache[x].block; // pointer to the block
        if (copyblock != NULL){
          int size = 256; 
          memcpy (copyblock, buf, size); // update 256 from buf to block
          return;
        }
      }
      return;
    }
    
  }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  if ( created == 0){ // fail if cache is not created.
    return -1;
  }
  if (cache == NULL){ // fail if empty cache
   return -1;
  }
  if (disk_num <0 || block_num<0){ //disk and block number can not be - number.
    return -1;
  }
 
  if (buf == NULL){ // buf can not be empty
    return -1;
  }
  if (disk_num >= 16 || block_num >= 256){ // total we have 16 disk and 256 block, no more.
    return -1;
  }
  else{
    int x;
    uint8_t *copyblock;
    //int lastuse = 0;
    for (x=0; x< cache_size; x++){
    
      if (disk_num == cache[x].disk_num && block_num == cache[x].block_num){
        if (cache[x].valid == true){
          printf("first.");
          return -1;
        }
        if (cache[x].valid == false){
          cache[x].valid = true;
          cache[x].disk_num = disk_num;
          cache[x].block_num = block_num;
          copyblock = cache[x].block;
          if (copyblock == NULL){
            return -1;
          }
          else{
            int size = 256;
            memcpy(copyblock, buf, size);
            fifoIndex ++;
            return 1;
          }
        }

      }
      if(disk_num != cache[x].disk_num && block_num != cache[x].block_num){
        if (cache[x].valid == true){
          cache[x].disk_num = disk_num;
          cache[x].block_num = block_num;
          copyblock = cache[x].block;
          if (copyblock == NULL){
            return -1;
          }
          else{
            int size = 256;
            memcpy(copyblock, buf, size);
            fifoIndex++;
            return 1;
          }
        }
        if (cache[x].valid == false){
          cache[x].valid = true;
          cache[x].disk_num =disk_num;
          cache[x].block_num = block_num;
          copyblock =cache[x].block;
          if (copyblock == NULL){
            return -1;
          }
          else{
            int size =256;
            memcpy (copyblock, buf, size);
            fifoIndex ++;
            printf("%d.", fifoIndex);
            return 1;
          }
        }
      }
      /*if (cache[x].valid == true){  // if this cache is already exist, no need to insert. first check valid cache.
        if (disk_num == cache[x].disk_num && block_num == cache[x].block_num){ // then disk block match one of x  inside cache, cache exist.
          printf("second");
          return -1;
        }
      }
      if(cache[x].valid == false){  // if no cache at all
        cache[x].valid = true; // set valid to true
        cache[x].disk_num = disk_num; // set given disknum/blocknum to cache
        cache[x].block_num = block_num; 
        copyblock = cache[x].block; // pointer point to cache block
        if (copyblock == NULL){
          return -1;
        }
        else{
          int size = 256;
          memcpy(copyblock, buf, size); // mem copy 256 from buf to block, insert to the cache.
          fifoIndex ++; // track how many insert we have done.
          //printf ("%d.", fifoIndex);
          return 1;
        }
        
      }
      else if (cache[x].valid == true ){ // if disk/block number do not match, but valid cache
        cache[x].disk_num = disk_num; // set given disknum/blocknum to cache
        cache[x].block_num = block_num;
        copyblock = cache[x].block; // pointer to the block
        if (copyblock == NULL){
          int size = 256; 
          memcpy(copyblock, buf, size); // memory copy block size from buf to block, success insert
          fifoIndex ++;  //track nuber of insertion we done.
          //printf ("%d.", fifoIndex);

          return 1;
        }
        
      }
      else {

      }*/
    }


  }    
  return -1;
}


bool cache_enabled(void) {
  if( cache_size >2){  // cache size minimum is 2, if bigger, return true.
    return true; 
  }
  else{
  return false;
  }
}

void cache_print_hit_rate(void) {
  fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
