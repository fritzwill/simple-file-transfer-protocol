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
#include<iostream>
#include<sstream>

#define MAX_LINE 256

void sendWithCheck(int, const void *, int, int);
int recvWithCheck(int, void *, int, int);
void handleLS(int, const void *, int, int);
void handleDL(int, const void *, int, int);

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

    printf("Connecting to %s on port %s\n", argv[1], argv[2]);

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
    
    printf("Connection established\n");
    printf("> ");
    // After connected, run main loop to get user inputs, send them to server, reacieve the results, then display
    int lenSendString;
    while (fgets(buf, sizeof(buf), stdin)){
        buf[MAX_LINE-1] = '\0'; // add null to end of buf to make c string
        lenSendString = strlen(buf) + 1;
        if (!strncmp(buf, "EXIT", 4)){
            sendWithCheck(sockFd, buf, lenSendString,0);  
            printf("Peace!\n");
            break; 
        }
        
        if (!strncmp(buf, "LS", 2)) handleLS(sockFd, buf, lenSendString, 0);
        else if (!strncmp(buf, "DL", 2)) handleDL(sockFd, buf, lenSendString, 0);
        printf("> ");
        fflush(stdout);
    }
    
    close(sockFd);
    printf("END\n");
    return 0;
} 

int recvWithCheck(int sockFd, void *buf, int len, int flags){
    int numBytesRec;
    if ((numBytesRec=recv(sockFd, buf, len, flags)) == -1){
        perror("receive error");
        close(sockFd);
        exit(1);
    }
    if (numBytesRec == 0){
        printf("recvWithCheck: zero bytes recieved\n");
        close(sockFd);
        exit(1);
    }
    return numBytesRec;
}

void sendWithCheck(int sockFd, const void *msg, int len, int flags){
    if (send(sockFd, msg, len, flags) == -1){
        perror("send error");
        close(sockFd);
        exit(1);
    }
}

void handleLS(int sockFd, const void *msg, int len, int flags){
    sendWithCheck(sockFd, msg, len, flags);
    int numBytesRec;
    int32_t fileSize;
    
    // read in how big listing is
    char *data = (char *)&fileSize;
    int remaining = sizeof(fileSize);
    int rc;
    while (remaining > 0){
        rc = recvWithCheck(sockFd, data, remaining, flags);
        data += rc;
        remaining -= rc;
    }

    // Now read in listing
    char buf[fileSize];
    remaining = fileSize;
    memset(buf, 0, sizeof(buf));
    std::string outStr = "";
    while (remaining > 0){
        rc = recvWithCheck(sockFd, buf, remaining, flags);
        outStr.append(buf);
        remaining -= rc;
    }
    outStr[fileSize] = '\0';
    printf("%s\r", outStr.c_str());
}

void handleDL(int sockFd, const void *msg, int len, int flags){
    sendWithCheck(sockFd, msg, len, flags); // let server know 
    std::string command((char *)msg);
    std::istringstream iss(command);
    std::string fileName;
    iss >> fileName; // this should be string 'DL'
    iss >> fileName; // this should be the full file path
    std::cout<<fileName<<std::endl;
    short int lenFile = fileName.length();
    char buf[BUFSIZ];
    sprintf(buf, "%d %s", lenFile, fileName.c_str());
    
    // sind file size and filename to server
    sendWithCheck(sockFd, buf, sizeof(buf), flags);

    // see what server says about that response
    int32_t fileSize;
    recvWithCheck(sockFd, &fileSize, sizeof(fileSize),0);
    if (fileSize < 0){
        printf("The file %s, does not exist on the server\n", fileName.c_str());
        return;
    }
    
    // now we need to receive md5 hash
    memset(buf, (char)NULL, sizeof(buf));
    recvWithCheck(sockFd, buf, sizeof(buf), 0);
    std::string serverHash(buf);
    std::cout<< serverHash <<std::endl;
    std::cout<< "File size: " << fileSize << std::endl;
/*                 Haven't got this to work yet
    // time to get the file
    FILE *receivedFile;
    receivedFile = fopen(fileName.c_str(), "w");
    if (receivedFile == NULL){
        printf("handleDL: failed to open new file to write to\n");
        exit(1);
    }
    int remainData = fileSize;
    memset(buf, (char)NULL, sizeof(buf));
    int len2;
    while (((len2 = recvWithCheck(sockFd, buf, sizeof(buf), 0)) > 0) && (remainData > 0)){
        fwrite(buf, sizeof(char), len2, receivedFile);
        remainData -= len;
    }
    FILE *md5Ptr;
*/
}
