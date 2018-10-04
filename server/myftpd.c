// Will Fritz
// wfritz
// sftp oserver 

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
#define BACKLOGN 10      // number of pending connections queue will hold

int main(int argc, char * argv[]){
    int sockFd;
    char buf[BUF_SIZE];
    int portReuse = 1; // yes, see setsockopt

    // check command line input
    if (argc != 2){
        fprintf(stderr, "You need two inputs, you provided: %d\n", argc);
        exit(1);
    }
    
    // load address structs
    struct addrinfo hints, *servInfo, *ptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // needs to be stream
    hints.ai_flags = AI_PASSIVE; // want to use my local IP, fill it in for me   

    // inputs: [IP of named addr, string(PORTNUM),struct addrinfo *,struct addrinfro **] 
    int returnVal;
    if ((returnVal = getaddrinfo(NULL, argv[1], &hints, &servInfo)) != 0){
        fprintf(stderr, "getaddrinfro: %s\n", gai_strerror(returnVal));
        exit(1);
    }

    // loop through getaddrinfo results and bind to first
    printf("before bind\n");
    for (ptr = servInfo; ptr != NULL; ptr = ptr->ai_next){
        // make socket
        if ((sockFd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1){
            perror("bad socket");
            continue;
        }
        // additional socket options
        if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &portReuse, sizeof(portReuse)) == -1){
            perror("bad setsockopt");
            exit(1);
        }  
        printf("after sock\n");
        // bind socket 
        if ((bind(sockFd, ptr->ai_addr, ptr->ai_addrlen) < 0)){
            close(sockFd);
            perror("bad bind");
            continue;
        }
        
        break; // break loop since we found a good bind
        printf("after bind\n");
    }

    free(servInfo); // don't need the struct anymore
    if (ptr == NULL){
        fprintf(stderr, "failed to bind\n");
        exit(1);
    }

    // listen on socket
    if ((listen(sockFd, BACKLOGN)) < 0){
        close(sockFd);
        perror("bad listen");
        exit(1);
    }

    // accept an incoming connection
    int newFd; // need a new file descriptor for accept, gets populated during call
    socklen_t addr_size;
    struct sockaddr_storage clientAddr; // gets populated in accept
    addr_size = sizeof(clientAddr);
    if ((newFd = accept(sockFd, (struct sockaddr *) &clientAddr, &addr_size)) < 0){
        perror("bad accept");
        exit(1);
    }
   
    // after accept, lets start main loop to interact with client
    int numBytesRec;
    while(1){
        if ((numBytesRec=recv(newFd, buf, sizeof(buf),0)) == -1){
            perror("recieve error");
            exit(1);
        }
        if (numBytesRec == 0) break; // no bytes recieved
        printf("Server recieverd: %s\n", buf);
    }
    
    close(sockFd);
    printf("END\n");
    return 0;
}

        
