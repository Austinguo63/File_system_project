#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "net.h"
#include "jbod.h"

/* the client socket descriptor for the connection to the server */
int cli_sd = -1;


/* attempts to read n (len) bytes from fd; returns true on success and false on failure. 
It may need to call the system call "read" multiple times to reach the given size len. 
*/
static bool nread(int fd, int len, uint8_t *buf) {
  int x;
  int y;
  for (x=0; x<len; x++){  //for loop, loop len times
  if (len == 0){  // len can  not be 0, since read will error.
    return false;
  }
  else{
    y = read(fd,buf,len); // use include header read to read from fd to buf
    if(y == len){  // if net read have same size with nread.
      return true;  //true
    }

  }
  }
  return false;
}

/* attempts to write n bytes to fd; returns true on success and false on failure 
It may need to call the system call "write" multiple times to reach the size len.
*/
static bool nwrite(int fd, int len, uint8_t *buf) {
  int x;
  int y;
  for (x=0; x<len; x++){  //loop len times
  if (len ==0){  //len can not be 0, else error
    return false;
  }
    y = write(fd,buf,len); //write len from fd to buf
    if (y == len){  // if same, true
      return true;
    }
  }
  return false;
}

/* Through this function call the client attempts to receive a packet from sd 
(i.e., receiving a response from the server.). It happens after the client previously 
forwarded a jbod operation call via a request message to the server.  
It returns true on success and false on failure. 
The values of the parameters (including op, ret, block) will be returned to the caller of this function: 

op - the address to store the jbod "opcode"  
ret - the address to store the return value of the server side calling the corresponding jbod_operation function.
block - holds the received block content if existing (e.g., when the op command is JBOD_READ_BLOCK)

In your implementation, you can read the packet header first (i.e., read HEADER_LEN bytes first), 
and then use the length field in the header to determine whether it is needed to read 
a block of data from the server. You may use the above nread function here.  
*/
static bool recv_packet(int sd, uint32_t *op, uint16_t *ret, uint8_t *block) {
  uint64_t size;  //length field
  uint8_t headerbuf[HEADER_LEN]; // array represents size of HEADERLEN

    memcpy(&size, headerbuf,2); // memory copy 2 bytes to length field.
    size = ntohs(size);
    memcpy(op, headerbuf+size, 4); //copy memory to opcode field.
    *op = ntohl(*op);
    memcpy(ret,headerbuf+*op, 2); //memcpy to return code field.
    if (264 != size){ //if we got actull size not equal to 256+8
      return false; 
    }
    uint8_t x[264];  //nread buf
    int y = nread(sd,JBOD_BLOCK_SIZE,x);
    if(size == 264 && y== true){  // if bytes equal 264, and nread is true
        memcpy(block,x, 256);// memcpy 256 bytes from nread buf to block
        return true;
      }
      else{
        return false;
      }
  return false;
}



/* The client attempts to send a jbod request packet to sd (i.e., the server socket here); 
returns true on success and false on failure. 

op - the opcode. 
block- when the command is JBOD_WRITE_BLOCK, the block will contain data to write to the server jbod system;
otherwise it is NULL.

The above information (when applicable) has to be wrapped into a jbod request packet (format specified in readme).
You may call the above nwrite function to do the actual sending.  
*/
static bool send_packet(int sd, uint32_t op, uint8_t *block) {
  uint8_t wrap[264];  // wrapbuf  of size 256+8
  uint64_t decode;  
  decode = htonl(op>>28); //we first decode op

  memcpy(&wrap[2], &decode,4); //wrap op

  uint8_t wraptemp;
  uint64_t writelen;

  if (decode == JBOD_WRITE_BLOCK){  //if command is JBODWRITEBLOCK,
    wraptemp = htons(HEADER_LEN + 263); //contain data to jbod
    int copybyte;
    copybyte = JBOD_BLOCK_SIZE + HEADER_LEN; //263
    memcpy(&wrap[8],block, copybyte); //copy block to wrap
    writelen = 263;  //BLOCKSIZE + HEADER
    memcpy(wrap, &wraptemp,2); //wrap all data
    int y = nwrite(sd,writelen, wrap); // only true when nwrite return true.
    if ( y != true){
      return false;
    }
    else{
      return true;
    }
  }
  return false;

}



/* attempts to connect to server and set the global cli_sd variable to the
 * socket; returns true if successful and false if not. 
 * this function will be invoked by tester to connect to the server at given ip and port.
 * you will not call it in mdadm.c
*/
bool jbod_connect(const char *ip, uint16_t port) {

  

  cli_sd = socket(AF_INET, SOCK_STREAM, 0);  //creating a socket follow calss ppt.
  if (cli_sd == -1){  //can not be -1
    printf("error on creat socket [%s]\n", strerror(errno));
    return false;
  }

  struct sockaddr_in caddr;  //follow class ppt set up server address.
  caddr.sin_family = AF_INET;  
  caddr.sin_port = htons(port);  // port to uin16_t port.
  if (inet_aton(ip, &caddr.sin_addr) == 0){  // if 0, false
    return false;
  }


  
  int y = sizeof(caddr);  //follow ppt step 3, connect to server.
  if (connect(cli_sd, (const struct sockaddr*) &caddr, y) == -1){ // if -1, bad connect
    return false;
  }
  if (connect(cli_sd, (const struct sockaddr*) &caddr, y) == 0){ //if 0, success
    return true;
  }
return false;
}



/* disconnects from the server and resets cli_sd */
void jbod_disconnect(void) {
  close(cli_sd);  //follow class note, close and set to -1.
  cli_sd = -1;
}



/* sends the JBOD operation to the server (use the send_packet function) and receives 
(use the recv_packet function) and processes the response. 

The meaning of each parameter is the same as in the original jbod_operation function. 
return: 0 means success, -1 means failure.
*/
int jbod_client_operation(uint32_t op, uint8_t *block) {
  if (send_packet(cli_sd, op, block) == true){  // if send function works, return0
    return 0;
  }
  else{
    return -1;  //else send fail
  }
  uint16_t retaddr;
  if (recv_packet (cli_sd,&op,&retaddr,block) == true){  // if recv == true, same
    return 0;  
  }
  else{  // fail
    return -1;
  }
    return -1;
}