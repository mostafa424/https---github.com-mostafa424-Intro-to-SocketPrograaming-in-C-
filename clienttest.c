/*
2 ** client.c -- a stream socket client demo
3 */

 #include <stdio.h>
 #include<string.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <errno.h>
 #include <netdb.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #define PORT "3490" // the port client will be connecting to
 #define MAXDATASIZE 100 // max number of bytes we can get at once
 // get sockaddr, IPv4 or IPv6:
 void *get_in_addr(struct sockaddr *sa)
 {
 if (sa->sa_family == AF_INET) {
 return &(((struct sockaddr_in*)sa)->sin_addr);
 }

 return &(((struct sockaddr_in6*)sa)->sin6_addr);
 }

 int main(int argc, char *argv[])
 {
 int sockfd, numbytes;
 char buf[MAXDATASIZE];
 struct addrinfo hints, *servinfo, *p;
 int rv;
 char s[INET6_ADDRSTRLEN];
 
 memset(&hints, 0, sizeof hints);
 hints.ai_family = AF_INET;
 hints.ai_socktype = SOCK_STREAM;

 if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
 fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
 return 1;
 }

 // loop through all the results and connect to the first we can
 for(p = servinfo; p != NULL; p = p->ai_next) {
 if ((sockfd = socket(p->ai_family, p->ai_socktype,
 p->ai_protocol)) == -1) {
 perror("client: socket");
 continue;
 }


 if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
 close(sockfd);
 perror("client: connect");
 continue;
 }

 break;
 }

 if (p == NULL) {
 fprintf(stderr, "client: failed to connect\n");
 return 2;
 }

 inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
 s, sizeof s);
 printf("client: connecting to %s\n", s);

 freeaddrinfo(servinfo); // all done with this structure
 char test []= "POST file is put successfully"; 
 (numbytes = send(sockfd, test, MAXDATASIZE-1, 0));
 char test2[30];
 (numbytes = recv(sockfd, test2, MAXDATASIZE-1, 0));
 for(int i=0;i<30;i++){
 printf("%c",test2[i]);
 } 
close(sockfd);
return 0;
 }