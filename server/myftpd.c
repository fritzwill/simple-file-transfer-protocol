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
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // needs to be stream
    hints.ai_flags = AI_PASSIVE; // want to use my local IP, fill it in for me   

    // inputs: [IP of named addr, string(PORTNUM),struct addrinfo *,struct addrinfro **] 
    getaddrinfo(NULL, argv[1], &hints, &res);

    printf("before bind\n");
    // make socket
    if ((sockFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("bad socket");
        exit(1);
    }
    // additional socket options
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &portReuse, sizeof(portReuse)) == -1){
        perror("bad setsockopt");
        exit(1);
    }
    
    printf("after sock\n");
    // bind socket 
    if ((bind(sockFd, res->ai_addr, res->ai_addrlen) < 0)){
        perror("bad bind");
        exit(1);
    }
    printf("after bind\n");
    
    // listen on socket
    if ((listen(sockFd, BACKLOGN)) < 0){
        perror("bad listen");
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
    printf("END\n");
    return 0;
}

        
