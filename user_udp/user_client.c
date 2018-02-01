#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "include/user_udp.h"

char * in_buf, * out_buf;
int sockfd,len, bytes_received;
struct sockaddr_in serv,cliaddr;

#if TEST == 1
  unsigned long long sent = 0, sent_min = 0, seconds = 0;
#endif

#if TEST == 2
  unsigned long long total = 0,counted = 0;
#endif

void sig_handler(int signo) {
  if (signo == SIGINT){
    close(sockfd);
    #if TEST == 1
      printf("Total number of sent packets: %llu\n", sent);
    #endif
    printf("Client closed\n");
  }
  exit(0);
}


int main(int argc,char *argv[]) {

  if(argc != 3){
    printf("Usage: %s ipaddress port\n",argv[0]);
    exit(0);
  }

  signal(SIGINT, sig_handler);

  if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0) {
    perror("error creating socket");
    exit(0);
  }

  // Make the recvfrom block for only 100 ms
  struct timeval t;
  t.tv_sec = 0;
  t.tv_usec = 100000;

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
             &t, sizeof(t));

  memset(&serv,0,sizeof(serv));
  serv.sin_family=AF_INET;
  serv.sin_port=htons(atoi(argv[2]));
  serv.sin_addr.s_addr=inet_addr(argv[1]);

  if((connect(sockfd, (struct sockaddr *)&serv,sizeof(serv))) < 0) {
    perror("ERROR connecting to server");
    exit(0);
  }

  printf("Client: Connected Successfully.\n");

  in_buf = malloc(MAX_UDP_SIZE);
  out_buf = malloc(MAX_MESS_SIZE);
  memcpy(out_buf, HELLO, strlen(HELLO)+1);

  #if TEST == 0
    // 0: send HELLO
    if((sendto(sockfd,out_buf,strlen(HELLO)+1,0,(struct sockaddr *)&serv,sizeof(serv)))<0) {
      perror("ERROR IN SENDTO");
    }
    printf("Client: sent HELLO\n");
  #else
    struct timeval departure_time,arrival_time;
    double average = 0;
    unsigned long long res;
    gettimeofday(&departure_time,NULL);
  #endif

  while(1){
    #if TEST != 0
      // 1: send forever
      // 2: send HELLO
      if((sendto(sockfd,out_buf,strlen(HELLO)+1,0,(struct sockaddr *)&serv,sizeof(serv)))<0) {
        perror("ERROR IN SENDTO");
      }
      #if TEST == 1
        else{
          sent_min++;
          gettimeofday(&arrival_time, NULL);
          res = (arrival_time.tv_sec * 1000000 + arrival_time.tv_usec) - (departure_time.tv_sec * 1000000 + departure_time.tv_usec );
          if(res >= 1000000){
            seconds ++;
            sent +=sent_min;
            average = (double)sent/(double)seconds;
            printf("Total package sent in a second: %lld \t Average %f/sec\n",sent_min,average );
            sent_min = 0;
            gettimeofday(&departure_time, NULL);
          }
        }
      #endif
    #endif

    #if TEST != 1
      // 0: receive OK, exit
      // 2: receive OK, resend
      memset(in_buf, 0, MAX_UDP_SIZE);
      bytes_received = recvfrom(sockfd,in_buf,MAX_UDP_SIZE,0,(struct sockaddr *)&cliaddr,&len);

      if(bytes_received == MAX_MESS_SIZE && memcmp(in_buf, OK, strlen(OK)+1) == 0){
        #if TEST == 2
          // 2: calculate LATENCY
          gettimeofday(&arrival_time,NULL);
          res = (arrival_time.tv_sec * 1000000 + arrival_time.tv_usec) - (departure_time.tv_sec * 1000000 + departure_time.tv_usec );
          total += res;
          counted ++;
          average = (double)total/ (double)counted;
          printf("\rLATENCY: %llu microseconds \t Average %.2f",res, average );
          gettimeofday(&departure_time,NULL);
        #else
          // 0: exit
          printf("Client: Received %s (%d bytes) from %s:%s\n", in_buf, bytes_received, argv[1], argv[2]);
          // printf("Client: Received %s from server\n",in_buf);
          break;
        #endif
      }
    #endif
  }

  return 0;

}
