// Will Fritz 
// wfritz
// 9/16/2018

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>

#define BUF_SIZE 4096

int main(int argc, char * argv[]){
    int sockFd;
    char buf[BUF_SIZE];

    // check command line input
    if (argc != 3){
        fprintf(stderr, "You need 3 inputs, you provided: %d\n", argc);
        exit(1);
    }

    // laod address structs
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // unputs: [IP or named addr, string(PORTNUM), struct addrinfo *, struct addrinfo **]
    getaddrinfo(argv[1], argv[2], &hints, &res);

    // make socket
    if ((sockFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("bad socket");
        exit(1);
    }    

    // make connect
    if (connect(sockFd, res->ai_addr, res->ai_addrlen) < 0){
        perror("bad connect");
        close(sockFd);
        exit(1);
    }

    printf("sjda\n");
    return 0;
} 
