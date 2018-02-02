#ifndef KERN_UDP
#define KERN_UDP

#include <asm/atomic.h>
#include <linux/udp.h>

/* 0 is only one echo message HELLO-OK
 * 1 is the THROUGHPUT test (client send, server receives)
 * 2 is the LATENCY test, (multiple echo message HELLO-OK)
*/
#define TEST 2

struct udp_service
{
  // struct socket * u_socket;
  struct task_struct * u_thread;
  char * name;
  atomic_t thread_running;  //1 yes 0 no
  atomic_t socket_allocated;//1 yes 0 no
};

#define MAX_RCV_WAIT 100000 // in microseconds
#define MAX_UDP_SIZE 65507
#define MAX_MESS_SIZE 6 // HELLO + \0
#define _100_MSEC 1000000

#define HELLO "HELLO"
#define OK "OK"

typedef struct udp_service udp_service;

extern void _send_message(struct socket * s, struct sockaddr_in * a, unsigned char * buff, int p, char * data, int len, char * module_name);
extern int udp_server_send(struct socket *sock, struct sockaddr_in *address, const char *buf, const size_t length, unsigned long flags, char * module_name);
extern int udp_server_receive(struct socket *sock, struct sockaddr_in *address, unsigned char *buf, unsigned long flags, udp_service * k);
extern u32 create_address(u8 *ip);
extern void udp_server_init(udp_service * k, struct socket ** s, unsigned char * myip, int myport);
extern void init_service(udp_service * k, char * name);
extern void check_sock_allocation(udp_service * k, struct socket * s);
extern void udp_server_quit(udp_service * k, struct socket * s);
extern void check_params(unsigned char * dest, unsigned int * src, int arg);


#endif
