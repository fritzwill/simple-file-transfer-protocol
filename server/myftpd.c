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
#include<dirent.h>
#include<sys/stat.h>
#include<stdint.h>
#include<string>
#include<iostream>
#include<sstream>
#include<fcntl.h>
#include<fstream>
#include<sys/sendfile.h>

#define MAX_LINE 256
#define BUF_SIZE 4096
#define BACKLOGN 10      // number of pending connections queue will hold

void sendWithCheck(int, const void *, int, int);
int recvWithCheck(int, void *, int, int);
void handleLS(int);
void handleDL(int);

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
        // bind socket 
        if ((bind(sockFd, ptr->ai_addr, ptr->ai_addrlen) < 0)){
            close(sockFd);
            perror("bad bind");
            continue;
        }
        
        break; // break loop since we found a good bind
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

    printf("Accepting connections on port %s\n", argv[1]); 

    // accept an incoming connection
    int newFd; // need a new file descriptor for accept, gets populated during call
    socklen_t addr_size;
    struct sockaddr_storage clientAddr; // gets populated in accept
    addr_size = sizeof(clientAddr);
    if ((newFd = accept(sockFd, (struct sockaddr *) &clientAddr, &addr_size)) < 0){
        perror("bad accept");
        exit(1);
    }
   
    printf("Connection established\n");
    
    // after accept, lets start main loop to interact with client
    int numBytesRec;
    while(1){
        memset(buf,0,sizeof(buf));
        numBytesRec = recvWithCheck(newFd, buf, sizeof(buf),0);
        if (numBytesRec == 0) break; // no bytes recieved
        printf("Server recieverd: %s", buf);
        if (!strncmp(buf, "EXIT", 4)){
            printf("EXIT command received from client, shutting down\n");
            break;
        }
        else if (!strncmp(buf, "LS", 2)) handleLS(newFd);
        else if (!strncmp(buf, "DL", 2)) handleDL(newFd);
        
    }
    
    close(sockFd);
    printf("END\n");
    return 0;
}

int recvWithCheck(int sockFd, void *buf, int len, int flags){
    int numBytesRec;
    if ((numBytesRec = recv(sockFd, buf, len, flags)) == -1){
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

void handleLS(int sockFd){
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    struct stat fileStat;
    std::string returnStr = "";
    if (d) {
        while ((dir = readdir(d)) != NULL){
            if (stat(dir->d_name, &fileStat) < 0){
                printf("handleLS: stat problem\n");
                exit(1);
            }
            else{
                returnStr.append((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                returnStr.append((fileStat.st_mode & S_IRUSR) ? "r" : "-");
                returnStr.append((fileStat.st_mode & S_IWUSR) ? "w" : "-");
                returnStr.append((fileStat.st_mode & S_IXUSR) ? "x" : "-");
                returnStr.append((fileStat.st_mode & S_IRGRP) ? "r" : "-");
                returnStr.append((fileStat.st_mode & S_IWGRP) ? "w" : "-");
                returnStr.append((fileStat.st_mode & S_IXGRP) ? "x" : "-");
                returnStr.append((fileStat.st_mode & S_IROTH) ? "r" : "-");
                returnStr.append((fileStat.st_mode & S_IWOTH) ? "w" : "-");
                returnStr.append((fileStat.st_mode & S_IXOTH) ? "x" : "-");
                returnStr = returnStr + " " + dir->d_name + "\n";
            }
        }
        int32_t sizeStr = returnStr.length();
        sendWithCheck(sockFd, (char *)&sizeStr, sizeof(sizeStr), 0);

        // Send ls
        sendWithCheck(sockFd, returnStr.c_str(), sizeStr,0);
        closedir(d);
    }
}

void handleDL(int sockFd){
    char buf[MAX_LINE];

    // parse client
    recvWithCheck(sockFd, buf, sizeof(buf), 0);
    std::string clientStr(buf);
    std::istringstream iss(clientStr);
    std::string fileName;
    short int lenFile;
    int32_t bytesInFile;
    int fd;
    iss >> lenFile;
    iss >> fileName;
    
    // deal with file
    FILE *fp;
    fp = fopen(fileName.c_str(), "r");
    if (fp){ // file exists
        FILE *md5Ptr;
        std::string command = "md5sum " + fileName;
        md5Ptr = popen(command.c_str(), "r");
        if (!md5Ptr){
            printf("handleDL: failed popen\n");
            exit(1);
        }
        memset(buf, (char)NULL, sizeof(buf));
        if (fread(buf, sizeof(char), sizeof(char)*sizeof(buf), md5Ptr) <0){
            printf("handleDL: failed fread\n");
            exit(1);
        }
        if (pclose(md5Ptr) < 0){
            printf("handleDL: pclose\n");
            exit(1);
        }
        printf("md5 is: %s\n", buf);

        struct stat s;
        fd = fileno(fp); // need file descriptor for fstat and sendfile
        if (fstat(fd, &s) == -1){
            perror("handleDL: fstat");
            exit(1);
        }
        bytesInFile = s.st_size;
        printf("file size: %d\n", bytesInFile);
        sendWithCheck(sockFd, &bytesInFile, sizeof(bytesInFile), 0);
        
    }
    else { // file does not exist
        int32_t returnVal = -1;
        sendWithCheck(sockFd, (char *)&returnVal, sizeof(returnVal), 0);
        return;
    }
    
    // sends hash
    sendWithCheck(sockFd, buf, sizeof(buf), 0);
    
/*              Haven't got this to work yet
    // send the file
    int remainData = bytesInFile;
    int sentBytes = 0;
    off_t offset = 0;
    while(((sentBytes = sendfile(sockFd, fd, &offset, BUFSIZ)) > 0) && (remainData > 0)){
        remainData -= sentBytes;
    }
   */ 
}
