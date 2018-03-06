#include "kernel_service.h"
#include "kernel_udp.h"
#include "k_file.h"

// Handles kthread initialization

struct file ** f;

struct udp_service {
  char name[15]; // same as p_name
  struct task_struct * k_thread;
  struct socket * sock;
};

void k_thread_stop(struct udp_service * k){
  if(k && k->k_thread)
    k->k_thread = NULL;
}


static void create_thread(struct udp_service * k, int(*funct)(void), void * data){
  size_t len = strlen(k->name);
  char name[len];
  memcpy(name, k->name, len);
  name[len-1] = '\0';
  k->k_thread = kthread_run((void *)funct, data, name);
  if(k->k_thread >= 0){
    printk(KERN_INFO "%s Thread running\n", k->name);
  }else{
    printk(KERN_ERR "%s Cannot run thread\n", k->name);
    k->k_thread = NULL;
  }
}


void init_service(udp_service ** k, char * name, unsigned char * myip, int myport, int(*funct)(void), void * data){
  *k = kmalloc(sizeof(struct udp_service), GFP_KERNEL);
  udp_service * service = *k;
  if(service){
    int len = strlen(name)+1 > 15 ? 15 : strlen(name)+1;
    memcpy(service->name, name, len);
    service->k_thread = NULL;
    service->sock = NULL;
    if(udp_init(&(service->sock), myip, myport) >=0){
      create_thread(service, funct, data);
    }
  }else{
    printk(KERN_ERR "Failed to initialize %s\n", name);
    *k = NULL;
  }
}


void quit_service(udp_service * k){
  int ret;
  if(k){
    if(k->k_thread){
      if((ret = kthread_stop(k->k_thread)) == 0){
        printk(KERN_INFO "%s Terminated thread\n", k->name);
      }else{
        printk(KERN_INFO "%s Error %d in terminating thread\n", k->name, ret);
      }
    }else{
      printk(KERN_INFO "%s Thread was already stopped\n", k->name);
    }

    release_socket(k->sock);
    printk(KERN_INFO "%s Released socket\n",k->name);
    printk(KERN_INFO "%s Module unloaded\n", k->name);
    kfree(k);
  }else{
    printk(KERN_INFO "Module was NULL, terminated\n");
  }
}


char * get_service_name(udp_service * k){
  return k->name;
}

struct socket * get_service_sock(udp_service * k){
  return k->sock;
}


void check_operation(enum operations * operation, char * op){
  if(op[0] == 'l' || op[0] == 'L'){
    *operation = LATENCY;
  }else if(op[0] == 't' || op[0] == 'T'){
    *operation = TROUGHPUT;
  }else if(op[0] == 'p' || op[0] == 'P'){
    *operation = PRINT;
  }else if(op[0] == 's' || op[0] == 'S'){
    *operation = SIMULATION;
  }else{
    printk(KERN_ERR "Opt %c not valid, using the default one\n", *op);
    *op = 'p';
  }
}


void check_params(unsigned char * dest, char * src){
  if(src != NULL)
    sscanf(src, "%hhu.%hhu.%hhu.%hhu",&dest[0], &dest[1], &dest[2], &dest[3]);
}


void adjust_name(char * print, char * src, int size_name){
  size_t len = strlen(src)+1 > size_name-3 ? size_name-3 : strlen(src)+1;
  memcpy(print, src, len);
  memcpy(print+len -1, ":", 2);
}


int prepare_files(enum operations op, unsigned int ntests){
  if(op != PRINT){
    f = kmalloc(sizeof(struct file *) * ntests, GFP_KERNEL);
    int n = 1;
    for (size_t i = 0; i < ntests; i++) {
      char filen[100];
      snprintf(filen, 100, "./results/kernel_data/results%u.txt", n);
      n *=2;
      file_close(file_open(filen, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU));
      f[i] = file_open(filen, O_CREAT | O_RDWR | O_APPEND, S_IRWXU);
      if(!(f[i])){
        printk(KERN_ERR "Cannot create file %s\n", filen);
        f[i] = NULL;
      }
    }

  }
  return 0;
}

void close_files(unsigned int nfiles){

  for (size_t i = 0; i < nfiles; i++) {
    if(f[i] != NULL)
      file_close(f[i]);
  }
  kfree(f);
}
