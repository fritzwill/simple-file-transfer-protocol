# simple-file-transfer-protocol
Uses C sockets to create a simple file transfer protocol (ftp) for a server and clients. The server is the filesytem from which we want to obtain files. The client simply gives various commands.

## Directory struture
**server/**
* Makefile
* myftpd.c

**client/**
* Makefile
* myftp.c

## Using the files
(run server first)
Server commands, run in server/ directory: 
```
$ make
$ ./myftpd [port_num]
Accepting connections on port [port_num]
```
Client commands, run in client/ directory: 
```
$ make
$ ./myftp [host_name] [port_num]
Connecting to [host_name] on port [port_num]
Connection established
>
 ```
 
## Current Funtionality
Given the cursor (>), you can currently run:
* LS - works 
* EXIT - works
* DL filename - detects when no file, sends hash from server to client, sends file length
