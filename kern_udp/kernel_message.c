#include "kernel_message.h"
#include "kernel_udp.h"

// related to throughput and latency tests. Shared between server and client
// check thread ps --ppid 2 -p 2 -o uname,pid,ppid,cmd,cls

int MAX_MESS_SIZE;
message_data * request;
message_data * reply;
message_data * test;

struct message_data{
  size_t mess_len;
  char mess_data[0];
};

message_data * create_message(size_t size_data, int recv){
  if(recv)
    size_data = MAX_UDP_SIZE;

  message_data * res = kmalloc(sizeof(struct message_data) + size_data, GFP_KERNEL);
  res->mess_len = size_data;
  return res;
}

char * get_message_data(message_data * mess){
  return mess->mess_data;
}


size_t get_message_size(message_data * mess){
  return mess->mess_len;
}


void delete_message(message_data * mess){
  if(mess)
    kfree(mess);
}

void init_default_messages(void){
  size_t size_req = strlen(REQUEST)+1;
  size_t size_repl = strlen(REPLY)+1;
  size_t size_test = strlen(TEST)+1;
  MAX_MESS_SIZE = max(max(size_req,size_repl), size_test);
  request = kmalloc(sizeof(message_data)+ MAX_MESS_SIZE, GFP_KERNEL);
  reply = kmalloc(sizeof(message_data)+ MAX_MESS_SIZE, GFP_KERNEL);
  test = kmalloc(sizeof(message_data)+ MAX_MESS_SIZE, GFP_KERNEL);

  request->mess_len = MAX_MESS_SIZE;
  reply->mess_len = MAX_MESS_SIZE;
  test->mess_len = MAX_MESS_SIZE;
  memset(reply->mess_data, '\0', MAX_MESS_SIZE);
  memset(request->mess_data, '\0', MAX_MESS_SIZE);
  memset(test->mess_data, '\0', MAX_MESS_SIZE);

  memcpy(reply->mess_data, REPLY, size_repl);
  memcpy(request->mess_data, REQUEST, size_req);
  memcpy(test->mess_data, TEST, size_test);
}


void del_default_messages(void){
  kfree(request);
  kfree(reply);
  kfree(test);
}


void fill_sockaddr_in(struct sockaddr_in * addr, unsigned char *  ip, int flag, int port){
  addr->sin_addr.s_addr = htonl(create_address(ip));
  addr->sin_family = flag;
  addr->sin_port = htons(port);
}


void division(size_t dividend, size_t divisor, char * result, size_t size_res){
  if(divisor > 0){
    snprintf(result, size_res, "%zu.%zu", dividend/divisor, (dividend*1000/divisor)%1000);
  }
}
