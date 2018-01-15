#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//this is the port we'll bind to.
#define PORTNO 25432
//Not for use in this file, crosslisted from client side (libnetfiles), for reference only:
//int errorArray[] = {HOST_NOT_FOUND, EACCES, EINTR, EISDIR,ENOENT,EROFS,ETIMEDOUT,EBADF,ECONNRESET,EPERM}; 


//helper function that thread calls
void* clientService (void* args){
	int sockfd = *(int*)args;
	int bytesrcvd = 0;
	int sizemsg = 0;
	char tempbuff[10];
	char command[3];
	char * buffer = NULL;
	char * pathname = NULL;
	char * returnmsg = NULL;
	int flags = -1;
	int fdlen =0;
	int msglen = 0;
	int i = 0;
	bzero(tempbuff,10);	
	//go through the loop until you reach a comma or until end of buffer
	do{
		bytesrcvd += read(sockfd,tempbuff+i,1);
		i++;
	}while(((tempbuff[i-1])!=',') && i < 10);


	//if no bytes received return error.

	if(bytesrcvd < 0){
		printf("error reading from socket.");
		returnmsg = (char*)malloc(10);
		bzero(returnmsg,10);
		returnmsg = "N2";
		msglen = 10;
		i = 0;
		int resultstatus = 0;
		while(i < msglen){
			resultstatus = write(sockfd,returnmsg+i,msglen);
			i += resultstatus;
		}
		if(resultstatus < 0){
			printf("Error writing to socket");
		 	exit(-1);
	 	}//end if 
	//	printf("Recieved this in the buffer: %s\n", buffer); 
		free(returnmsg);
		pthread_exit(NULL);
		close(sockfd);
		return;
	}//end if

	
	//if got to end of buffer without a comma, bad format.
	if(tempbuff[i-1]!=','){
		printf("Not a valid message.\n");
		returnmsg = (char*)malloc(10);
		bzero(returnmsg,10);
		returnmsg = "N9";
		msglen = 10;
		int resultstatus = 0;
		while(i < msglen){
			resultstatus = write(sockfd,returnmsg+i,msglen);
			i += resultstatus;
		}
		
	 	if(resultstatus <= 0){
	 		printf("Error writing to socket");
		 	exit(-1);
	 	}//end if 
		free(returnmsg);
		pthread_exit(NULL);
		close(sockfd);
		return;
	}
	//if everything else worked make the last character a '\0';
	tempbuff[i-1]='\0';
	//now that we stored the size of the message, change it to a number		
	sizemsg = atoi(tempbuff);
	//malloc a buffer the size of the message.
	buffer = (char*)malloc(sizemsg+1);
	bzero(buffer,sizemsg+1);

	i = 0;
	//receive the rest of the message into that malloced buffer
	while(i < sizemsg && bytesrcvd != -1){
		bytesrcvd = read(sockfd,(void*)buffer+i,sizemsg);
		i += bytesrcvd;
	}
	buffer[sizemsg] = '\0';;
	
//	printf("Recieved this in the buffer: %s\n", buffer); 

	//store command and then do if else.
	command[0] = buffer[0];
	command[1] = buffer[1];
	command[2] = '\0';	
	
	/* * * * * * * * * * * * *

		OPEN FUNCTION STUFF
		//add the flags

	* * * * * * * * * * * * */


	if(strcmp(command,"OP")==0){
		printf("Request to open ");
		//grab index 2, that's the flag
		int flag = buffer[2];
		//malloc the rest of the path.
		pathname = (char*)malloc(sizemsg - 3);
		bzero(pathname,sizemsg - 3);
		pathname = strcpy(pathname,buffer+3);
		printf("%s\n",pathname);
		int fd = open(pathname,flag);
		free(pathname);
		//Return errno to client side
		if(fd < 0){
			if(errno == EACCES){
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N1";
				perror("");
				msglen = 3;					
			}
			else if(errno == EINTR){	
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N2";
				perror("");
				msglen = 3;					
			}
			else if (errno == EISDIR){
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N3";
				perror("");
				msglen = 3;					
			}
			else if (errno == ENOENT){
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N4";
				perror("");
				msglen = 3;					
			}
			else if(errno == EROFS){
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N5";
				perror("");
				msglen = 3;					
			}
			else{
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N6";
				msglen = 3;					
			}
		}
		else{
			fdlen = floor(log10(abs(fd)))+1;
		
			returnmsg = (char*)malloc(10);
			bzero(returnmsg,10);
			returnmsg[0]= 'Y';
			sprintf(returnmsg+1, "%d", fd*-1);
			returnmsg[fdlen+2] = '\0';
		}
		msglen = 10;
	}

	/* * * * * * * * * * * * *

		READ FUNCTION STUFF

	* * * * * * * * * * * * */

	else if (strcmp(command,"RD")==0){
		printf("Request to read ");
		
		char strfd[10]; 
		fdlen = -1;
		i = 2;

		//get length of fd
		while((buffer[i-1] != ',') && i < 10){
			fdlen++;
			i++;
		}
		
		i = 0;
		//put fd into a string
		while(i < fdlen){
			strcpy(strfd+i,buffer+i+2);
			i++;
		}
		strfd[fdlen] = '\0';
		//make fd a number
		int fd = atoi(strfd)*-1;
		printf("from fd %d\n", fd*-1);

		if(fd <= 1){
			returnmsg = (char*)malloc(10);
			bzero(returnmsg,10);
			returnmsg = "N7";
			msglen = 10;
			i = 0;
			int resultstatus = 0;
			while(i < msglen){
				resultstatus = write(sockfd,returnmsg+i,msglen);
				i += resultstatus;
			}
			if(resultstatus < 0){
				printf("Error writing to socket");
			 	exit(-1);
		 	}//end if 
			pthread_exit(NULL);
			close(sockfd);
			return;				
		}
		
		//start after RD and fd, set i to next character.
		i = 2+fdlen;
		
		//get length of size, put into string, then turn into number
		char* sizebytes = (char*)malloc(sizemsg - 2 -fdlen+1);
		bzero(sizebytes,sizemsg-2-fdlen+1);
		strcpy(sizebytes,buffer+2+fdlen+1);
		int bytes = atoi(sizebytes);
		free(sizebytes);
		
		//malloc enough space for the bytes requested plus 1 more for null terminator
		char * msg = (char*)malloc(bytes+1);
		bzero(msg,bytes+1);	
		int bytesread = -2;
		int total = 0;

		//read requested amount of bytes into buffer
		while(bytesread != 0 && bytesread != -1 && total < bytes){
			bytesread = read(fd, (void*)msg+total, bytes);
			total += bytesread;
		}
		msg[bytes] = '\0';		
		//get length of messageLength (if message is 20 bytes long, length of 20 is 2)
		int messageLength = floor(log10(abs(total)))+1;
		
		//malloc enough space for the message length plus the Y for the front and the comma for after
		char * totalstr = (char*)malloc(messageLength+2); //for ',' and '\0'
		bzero(totalstr,messageLength+2);
		sprintf(totalstr,"%d,",total);

		//put it all together in the return message
		returnmsg = (char*)malloc(strlen(totalstr)+strlen(msg)+2);//one for 'Y', one for '/0'
		bzero(returnmsg,strlen(totalstr)+strlen(msg)+2);
		returnmsg[0] = 'Y';
		returnmsg[strlen(totalstr)+strlen(msg)+2]='\0';
		strcpy(returnmsg+1,totalstr);
		strcat(returnmsg,msg);		
		free(msg);

		//error number time		
		if(bytesread == -1){
			free(totalstr);
			if(errno == EBADF){
				perror("");
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N7";	
				msglen = 3;					
			}
			else{
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N6";
				msglen = 3;
			}
		}
		else{
			//set return message stuff to client.
			returnmsg[bytes+1+strlen(totalstr)] = '\0';
			msglen = strlen(returnmsg);
			free(totalstr);

		}
		
	}
	
	/* * * * * * * * * * * * *

		WRITE FUNCTION STUFF

	* * * * * * * * * * * * */

	else if (strcmp(command,"WR")==0){
		printf("Request to write ");		
		char strfd[10]; 
		fdlen = -1;
		i = 2;
		
		//get length of fd
		while((buffer[i-1] != ',') && i < 10){
			fdlen++;
			i++;
		}
		i = 0;
		
		//put it into string
		while(i < fdlen){
			strfd[i]=buffer[i+2];
			i++;
		}
		strfd[fdlen] = '\0';
		
		//turn it into number
		int fd = atoi(strfd)*-1;

		printf("from fd: %d\n", fd*-1);

		if(fd <= 1){
			returnmsg = (char*)malloc(10);
			bzero(returnmsg,10);
			returnmsg = "N7";
			msglen = 10;
			i = 0;
			int resultstatus = 0;
			while(i < msglen){
				resultstatus = write(sockfd,returnmsg+i,msglen);
				i += resultstatus;
			}
			if(resultstatus < 0){
				printf("Error writing to socket");
			 	exit(-1);
	 		}//end if 
			pthread_exit(NULL);
			close(sockfd);
			return;
		}

		int sizelen = 0;
		
		char strsize[10];
		bzero(strsize,10);
		//get length of string size
		i = 2+fdlen+1; //this leaves off at first character of size
		do {
			sizelen++;
			i++;						
		}while((buffer[i-1]!= ',') && sizelen < 10);
		
		i=0;
		//put it into buffer
		//
		while(i < sizelen){
			strsize[i] = buffer[i+2+fdlen+1];
			i++;
		}
		strsize[sizelen] = '\0';

		//make size into number
		int size = atoi(strsize);
		//malloc space for a write buffer and put the stuff to write inside of it.
		char * writebuff = (char*)malloc(size+1);//1 for 'Y'; 1 for ','; 1 for '\0'
		bzero(writebuff,size+1);
		strncpy(writebuff, buffer+2+fdlen+1+sizelen,size); //2 for WR, fdlen, 1 for ',', sizelen includes extra comma.
		writebuff[size]='\0';	
		
		int resultstatus = 0;
		int i = 0;
		//write to file
		while(i < size && resultstatus != -1){
			resultstatus = write(fd,(const void*)writebuff+i,size);
			i += resultstatus;
		}		
		free(writebuff);

		//STOP!  Errno time!		
		if(resultstatus == -1){
			if(errno == EBADF){
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N7";	
				msglen = 3;	
			}
			else{
				returnmsg = (char*)malloc(10);
				bzero(returnmsg,10);
				returnmsg = "N6";
				msglen = 3;
			}
		}
		else{
			//set return message stuff to client.
			returnmsg = (char*)malloc(10);
			bzero(returnmsg,10);
			returnmsg[9] = '\0';
			returnmsg[0] = 'Y';
			sprintf(returnmsg+1, "%d", resultstatus);
		
			msglen = 10;
		}	
	}

	/* * * * * * * * * * * * *

		CLOSE FUNCTION STUFF

	* * * * * * * * * * * * */

	else if (strcmp(command,"CL")==0){
		printf("Request to close ");
		
		//take fd, save to string, then make number.
		char* strfd = (char*)malloc(sizemsg - 2 +1); 
		bzero(strfd,sizemsg-2+1);
		strcpy(strfd,buffer+2);
		strfd[sizemsg-2] = '\0';

		int fd = atoi(strfd)*-1;
		free(strfd);
		printf("fd %d\n",fd*-1);

		if(fd <= 1){
			returnmsg = (char*)malloc(10);
			bzero(returnmsg,10);
			returnmsg = "N7";
			msglen = 3;
			i = 0;
			int resultstatus = 0;
			while(i < msglen){
				resultstatus = write(sockfd,returnmsg+i,msglen);
				i += resultstatus;
			}
			if(resultstatus < 0){
				printf("Error writing to socket");
			 	exit(-1);
	 		}//end if 
//			free(returnmsg);
			pthread_exit(NULL);
			close(sockfd);
			return;
		}

		//close it up
		int status = close(fd);
		
		//one last errno, for old time's sake.
		if(status == -1){
			returnmsg = (char*)malloc(3);
			bzero(returnmsg,10);
			returnmsg = "N7";
			msglen = strlen(returnmsg);
		}
		else{
			//return to the client.
			returnmsg = (char*)malloc(2);
			bzero(returnmsg,10);
			returnmsg = "Y";	
			msglen = strlen(returnmsg);	
		}
	}	

	/* * * * * * * * * * * * *

		BIG ERROR STUFF

	* * * * * * * * * * * * */

	else{
		printf("Super not good - bad command\n");
	}	


	/* * * * * * * * * * * * *

		RETURNING THE MESSAGE

	* * * * * * * * * * * * */

	//write back to client
	int resultstatus = 0;
	i=0;
	while(i < msglen){
		resultstatus = write(sockfd, returnmsg+i, msglen);
		i += resultstatus;
	}
	
	printf("Returning message to client %s\n",returnmsg);	
 	if(resultstatus < 0){
 		printf("Error writing to socket");
 		perror("");

	 	exit(-1);
	 	//add errno stuff.
 	}//end of 
// 	free(returnmsg);
	pthread_exit(NULL);
	close(sockfd);

	return;
}


int main(int argc, char** argv){

	if(argc > 1){
		printf("This program does not take parameters, please try again without parameters\n");
		exit(-1);
	}

	int sockfd;
	struct sockaddr_in serverAddressInfo, clientAddressInfo;
	int option = 1; //changed from 1 to true since I defined it as an enum.
	socklen_t clilen = sizeof(struct sockaddr_in);
//	char buffer[BUFF_SIZE];
	int socketCounter = 0;//will this become the netfd?  If so, start at 2 to avoid -1.  Else start at 0. //Definitely start at 0.
	pthread_t threadArray[1000];//bigger? Instead of array malloc a linked list?  MALLOC :(
    
	sockfd = socket(AF_INET , SOCK_STREAM , 0);
	
	if(sockfd == -1){
		printf("Error in socket.\n");
		return -1;
		//add errno stuff
	}	
	
	//socket stuff.
	bzero((char*)&serverAddressInfo, sizeof(serverAddressInfo));
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_port = htons(PORTNO);
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sockfd, (struct sockaddr *) &serverAddressInfo, sizeof(serverAddressInfo)) == -1){
		printf("Error in bind\n");
		return -1;
	}
	
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char*)&option,sizeof(option));
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,(char*)&option,sizeof(option));
	setsockopt(sockfd, SOL_TCP, TCP_NODELAY, (char*)&option,sizeof(option));
	 
	 //infinite loop to listen
	while(1){ 
		listen(sockfd, 25);  //test before and after while later.
	
		printf("Listening for new connections... \n");	
		int newsock = accept(sockfd, (struct sockaddr*)&clientAddressInfo, &clilen);

		if(newsock < 0){
			printf("error creating new socket");
			return -1;
		} //end if
		
		int * socketptr = (int*)malloc(sizeof(int*));
		bzero(socketptr,sizeof(int*));
		*socketptr = newsock;
		
		printf("Accepted connection\n");
				
		//pass to new thread.
		int threadstatus = pthread_create(&threadArray[socketCounter],NULL,clientService,(void*)socketptr);
	

		if(threadstatus == -1){
			printf("Error in client Service\n");
			return -1;
		}

		socketCounter++;
	}//end while	
	return 0;
	
}//end main

