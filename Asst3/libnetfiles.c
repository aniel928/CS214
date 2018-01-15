#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/tcp.h>
#include "libnetfiles.h"


int errorArray[10] = {HOST_NOT_FOUND, EACCES, EINTR, EISDIR, ENOENT, EROFS, ETIMEDOUT, EBADF, ECONNRESET, EPERM};


struct hostent * serverIPAddress;
char* hostName;


int initialized = 0;
char servipaddr[20];


//Connect to socket and return socket fd
int getSocket(struct sockaddr_in serverAI){
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&serverAI, sizeof(serverAI)); 		
	serverAI.sin_family = AF_INET;
	serverAI.sin_addr.s_addr = inet_addr(servipaddr); 		
	serverAI.sin_port = htons(PORTNO);
	errno = 0;
	connect(socketfd, (struct sockaddr *) &serverAI, sizeof(serverAI));
	int option = 1;
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, (char*)&option, sizeof(option));
	setsockopt(socketfd, SOL_TCP, TCP_NODELAY, (char*)&option, sizeof(option));
	return socketfd;
	}

int netopen(const char *pathname, int flags){
	//check if initialized
	if(initialized==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}
	if (flags != O_WRONLY && flags != O_RDONLY && flags != O_RDWR){
		errno = EINVAL;
		return -1;
	}
	
	//creating message to send
	int buff_size = strlen(pathname) + 2 + 1;	//+2 for OP, +1 for flag number
 	int bSize = floor(log10(abs(buff_size)))+1;	//get char length of string size
	char buffer[buff_size + 1 + bSize];		//char array size of enough chars for size of string + , + string
	bzero(buffer, buff_size + 1 + bSize);		//zer out array
	
	//put entire string into buffer
	sprintf(buffer, "%d,OP%d%s", buff_size, flags, pathname);
	
	//connecting to socket and getting sockfd
	struct sockaddr_in serverAddressInfo; 	
	int sockfd = getSocket(serverAddressInfo);
	
	int bytesrcvd = 0; 
	int i=0;

	//loop for ensuring everything is sent
	while(i< strlen(buffer) && bytesrcvd !=-1){
		bytesrcvd = write(sockfd, (void*)buffer +i, strlen(buffer));
		i+=bytesrcvd;
	}
	if(bytesrcvd == -1){
		return -1;
	}
	
	char returnStr[40];
	bzero(returnStr, 40);
	bytesrcvd = 0;

	//Read in first char to check if it is "Y" or "N"
	char* firstChar = (char*)malloc(2);
	bytesrcvd = read(sockfd, (void*)firstChar, 1);
	firstChar[1] = '\0';
	i =0;
	
	//loop for reading all of bytes sent back 
	while(i < 9 && bytesrcvd != -1){	
		bytesrcvd = read(sockfd, (void*)returnStr+i, 9);
		i += bytesrcvd;
	}
	if(bytesrcvd == -1){
		return -1;
	}
	if(strcmp(firstChar, "N") == 0){
		errno = errorArray[atoi(returnStr+0)];
		close(sockfd);
		free(firstChar);
		return -1;
	}else{	
		close(sockfd);
		free(firstChar);
		return atoi(returnStr);
	}	
}
ssize_t netread(int filedes, void* buf, size_t nbyte){
	//check if initialized	
	if(initialized==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}

	//compute size of sending string
	int fdSize = floor(log10(abs(filedes))) + 2;
	int nbSize = floor(log10(abs(nbyte))) +1;
	int total = 2 + fdSize + 1 + nbSize;
	int rSize = floor(log10(abs(total))) +1;

	//connect and create socket
	struct sockaddr_in serverAddressInfo; 	
	int sockfd = getSocket(serverAddressInfo);

	//build sending string
	char sendStr[rSize + 1 + 2 + fdSize + 1 + nbSize];
	sprintf(sendStr, "%d,RD%d,%d", total,filedes, nbyte);
	
	int bytesrcvd = 0;
	int i = 0;

	//make sure that entire string is sent
	while (i < strlen(sendStr) && bytesrcvd != -1){
		bytesrcvd =  write(sockfd, (void*) sendStr+i, strlen(sendStr));
		i += bytesrcvd;
	}
	if(bytesrcvd == -1){
		return -1;
	}

	bytesrcvd = 0;	
	char* msg = NULL;

	//malloc space for first char in sting
	char* firstChar = (char*) malloc(2);
	bytesrcvd = read(sockfd, (void*)firstChar, 1);
	firstChar[1] = '\0';	

	//space for reading in number
	char tempbuff[10];
	bzero (tempbuff,10);

	//Check first char for "Y" or "N"
	if(strcmp(firstChar, "Y") ==0){		
		i = 0;
		//read until >,< is found and put it all into a temp buffer
		do {
			bytesrcvd += read(sockfd, tempbuff+i,1);
			i++;
		}while(((tempbuff[i-1])!=',') && i< 10);
		tempbuff[i-1] = '\0';		//add null terminator
		int  sizemsg =  atoi(tempbuff);
		bytesrcvd = -2;
		i=0;
		msg = (char*) malloc(sizemsg+1);
		
		//loop for reading back entire returned message
		while(i < sizemsg && bytesrcvd != -1){
			bytesrcvd = read(sockfd, msg+i, sizemsg);
			i += bytesrcvd;
		}
		if(bytesrcvd == -1){
			return -1;
		}	
		memcpy(buf, msg, sizemsg);	//copy message received into buffer
		msg[sizemsg] = '\0'; 		//add null terminator to end of message
		close(sockfd);	
		int returnSize = strlen(msg);
		free(firstChar);
		free(msg);		
		return returnSize;
	}
	else{
		char* errNum = (char*) malloc(1);	
		read(sockfd, (void*)errNum, 1);
		errno = errorArray[atoi(errNum)];
		free(firstChar);
		return -1;
	}
}

ssize_t netwrite (int filedes, const void* buf, size_t nbyte){
	//check if initialized
	if(initialized==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}

	//compute size of string
	int fdSize = floor(log10(abs(filedes))) + 2;
	int nbSize = floor(log10(abs(nbyte))) +1;
	int total = 2 + fdSize + 1 + nbSize + 1 + strlen((char*)buf);
	int rSize = floor(log10(abs(total))) +1;

	//connect and create socket
	struct sockaddr_in serverAddressInfo; 	
	int sockfd = getSocket(serverAddressInfo);

	//create char array for correct size and build sending message
	char sendStr[rSize + 1 + 2 + fdSize + 1 + nbSize + 1 + strlen((char*)buf)];
	sprintf(sendStr, "%d,WR%d,%d,%s", total,filedes, nbyte, buf);

	int bytesrcvd = 0;
	int i =0;

	//loop for sending message and making sure entire string is sent
	while (i < nbyte && bytesrcvd !=-1){
		bytesrcvd = write(sockfd, (void*)sendStr +i, strlen(sendStr));
		i +=bytesrcvd;
	}
	if(bytesrcvd == -1){
		return -1;
	}

	//read in first character of returned message
	char* firstChar = (char*)malloc(2);
	char* msg;
	read(sockfd, (void*) firstChar, 1);
	firstChar[1] = '\0';

	//check if first character is "Y"
	if(strcmp(firstChar, "Y") == 0){
		msg = (char*)malloc(10);
		i = 0;
		while(i<9 && bytesrcvd != -1){
			bytesrcvd = read(sockfd, (void*) msg+i, 9);
			i += bytesrcvd;
		}
		if(bytesrcvd ==-1){
			return -1;
		}
		close(sockfd);
		int returnNum = atoi(msg);
		free(firstChar);
		free(msg);
		return returnNum;
	}
	//returned error message
	else{
		char* errNum = (char*)malloc(1);
		read(sockfd, (void*) errNum, 1);
		errno = errorArray[atoi(errNum)];
		free(firstChar);
		free(errNum);
		close(sockfd);
		return -1;
	}
}

int netclose (int fd){
	
	//check if initialize 
	if(initialized==0){
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}
	
	//create and connect to socket
	struct sockaddr_in serverAddressInfo; 	
	int sockfd = getSocket(serverAddressInfo);
	
	//compute size of sending string and build
	int fdSize = floor(log10(abs(fd))) +2;	//adding extra 1 for negative 
	int total = 2 + fdSize;
	int rSize = floor(log10(abs(total))) + 1;
	char sendStr [rSize + 1 + fdSize];
	sprintf(sendStr, "%d,CL%d", total, fd);

	int bytesrcvd=0;
	int i =0;
	
	//loop to make sure entire string is sent
	while (i<strlen(sendStr) && bytesrcvd != -1){
		bytesrcvd = write(sockfd, (void*)sendStr+i, strlen(sendStr));
		i+=bytesrcvd;
	}
	if(bytesrcvd == -1){
		return -1;
	}

	//check first character of returned message
	char* firstChar = (char*)malloc(2);
	read(sockfd, (void*) firstChar, 1);	
	firstChar[1] = '\0';
	
	//exit without error if "Y" is sent back, otherwise return -1 and set errno to appropriate error number
	if(strcmp(firstChar, "Y")==0){
		close(sockfd);
		free(firstChar);
		return 0;
	}
	else{
		char* errNum = (char*)malloc(1);
		read(sockfd, (void*) errNum, 1);
		errno = errorArray[atoi(errNum)];
		close(sockfd);
		free(firstChar);
		free(errNum);
		return -1;
	}
}

int netserverinit(char * hostname){
	
	int sockfd;  
	struct addrinfo addr, *servaddr, *i;
	bzero(&addr, sizeof(addr));
	addr.ai_family = AF_UNSPEC; 
	addr.ai_socktype = SOCK_STREAM;
	
	//checking if given hostname is valid
	int status = getaddrinfo(hostname, NULL, &addr, &servaddr);
	if(status != 0) {
		h_errno = HOST_NOT_FOUND;
		printf("%s\n", hstrerror(h_errno));
		return -1;
	}
	
	//loop for getting info
	char host[256], service[256];
	i = servaddr;
	while (i != NULL){
		getnameinfo(i->ai_addr, i->ai_addrlen, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST);
		i = i->ai_next;
	}

	//set initialized value to 1
	initialized = 1;
	strcpy(servipaddr, host);
	return 0;
}