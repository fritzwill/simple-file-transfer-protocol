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

#define MAX_LINE 256

int main(int argc, char * argv[]){
    int sockFd;
    char buf[MAX_LINE];

    // check command line input
    if (argc != 3){
        fprintf(stderr, "You need 3 inputs, you provided: %d\n", argc);
        exit(1);
    }

    // laod address structs
    struct addrinfo hints, *clientInfo, *ptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // unputs: [IP or named addr, string(PORTNUM), struct addrinfo *, struct addrinfo **]
    int returnVal;
    if ((returnVal = getaddrinfo(argv[1], argv[2], &hints, &clientInfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", returnVal);
        exit(1);
    }

    // loop through getaddrinfo results and connect to first we can
    for (ptr = clientInfo; ptr != NULL; ptr = ptr->ai_next){
        // make socket
        if ((sockFd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0){
            perror("bad socket");
            continue;
        }    

        // make connect
        if (connect(sockFd, ptr->ai_addr, ptr->ai_addrlen) < 0){
            perror("bad connect");
            close(sockFd);
            continue;
        }
        break; //found valid connect
    }
    
    free(clientInfo); // don't need struct anymore
    if (ptr == NULL){
        fprintf(stderr, "failed to connect\n");
        exit(1);
    }
    
    // After connected, run main loop to get user inputs, send them to server, reacieve the results, then display
    int lenSendString;
    while (fgets(buf, sizeof(buf), stdin)){
        buf[MAX_LINE-1] = '\0'; // add null to end of buf to make c string
        if (!strncmp(buf, "EXIT", 4)){
            printf("Peace!\n");
            break; 
        }
        lenSendString = strlen(buf) + 1;
        if (send(sockFd ,buf, lenSendString, 0) == -1){
            perror("client send error");
            close(sockFd);
            exit(1);
        }
    }
    
    close(sockFd);
    printf("END\n");
    return 0;
} 
