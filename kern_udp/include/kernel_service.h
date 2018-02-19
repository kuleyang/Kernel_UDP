#ifndef K_SERVICE
#define K_SERVICE

#include <net/sock.h>
#include <linux/kthread.h>

#define SIZE_NAME 20

enum operations {
  PRINT,
  TROUGHPUT,
  LATENCY
};

typedef struct udp_service udp_service;

extern void k_thread_stop(struct udp_service * k);

extern void init_service(udp_service ** k, char * name, unsigned char * myip, int myport, int(*funct)(void), void * data);
extern void quit_service(udp_service * k);
extern char * get_service_name(udp_service * k);
extern struct socket * get_service_sock(udp_service * k);

extern void check_operation(enum operations * operation, char * op);
extern void check_params(unsigned char * dest, unsigned int * src, int arg);
extern void adjust_name(char * print, char * src, int size_name);

extern int prepare_file(enum operations op);


#endif
