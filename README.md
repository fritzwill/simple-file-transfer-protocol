# simple-file-transfer-protocol
Uses C sockets to create a simple file transfer protocol (ftp) server and clients

The file setup:
**server/**
* Makefile
* myftpd.c

**client/**
* Makefile
* myftp.c

(run server first)
server command: myftpd 41016

client command: myftp student01.cse.nd.edu 41016
 >
 
Given the cursor, you can currently run:
LS - works 
EXIT - works
DL filename - detects when no file, sends hash from server to client, sends file length

Also, there is commented out code at then end. I thin that is the correct approach to both send from the server and receive at the client. If i run the programs with a valid filename, then the client downloads part of the file (16K vs 24K)


