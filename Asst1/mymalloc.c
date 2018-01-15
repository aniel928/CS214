#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mymalloc.h"

metadata *header = NULL;
metadata *currentMeta = NULL;
metadata *previousMeta = NULL;

static char memory[MEMORYSIZE];

char * mymalloc(int numOfBytes, char * myfile, int myline){
	//Asking for 0 bytes
	if(numOfBytes == 0){
		printf("No bytes requested, file: %s, line: %d\n", myfile, myline);
		return NULL;
	}
	//very first malloc call
    if(header == NULL){
    	if(numOfBytes > (MEMORYSIZE - sizeof(metadata))){//check to make sure not more than size of array
			printf("Not enough memory available to allocate %d bytes. File: %s, Line: %d\n",numOfBytes,myfile,myline);
			return NULL;
		}
		else{
			//set up header
			header = (metadata*) memory;
			header->size = 0;
			header->ptr = memory+sizeof(metadata);//point to end of this metadata
			header->free = false;
	
			//set up first metadata and return pointer to data
			currentMeta = (metadata*)(header->ptr);//start of metadata for byte request
			currentMeta->size = numOfBytes;
			currentMeta->ptr = ((char*)currentMeta+sizeof(metadata)+numOfBytes);//point to end of users data
			currentMeta->free = false;
	
			char * userPointer = ((char*)currentMeta+sizeof(metadata)); //point to beginning of users data

			return userPointer;
		}
	}
	//Every call after the first call for malloc()
	currentMeta = header;
	currentMeta->ptr = header->ptr;
	
	//not sure this will ever happen - can probably remove, but just in case the header pointer gets nulled out, this will fix it.
	if(header->ptr == NULL){
		printf("Will this ever happen?\n");
		currentMeta = (metadata*)(header+sizeof(metadata));//start of metadata for byte request
		currentMeta->size = numOfBytes;
		currentMeta->ptr = ((char*)currentMeta+sizeof(metadata)+numOfBytes);//point to end of users data
		currentMeta->free = false;	
		
		char * userPointer = ((char*)currentMeta+sizeof(metadata)); //point to beginning of users data

		return userPointer;
	} 
		
	while(currentMeta->ptr != NULL){
		//check to make sure it won't walk off end of array
		if((((long)currentMeta + sizeof(metadata) + currentMeta->size +sizeof(metadata) + numOfBytes) - (long)memory)>MEMORYSIZE){
			printf("Not enough memory available to allocate %d bytes. File: %s, Line: %d\n",numOfBytes,myfile,myline);			
			return NULL;
		}
		int sizeBtwn = ((currentMeta->ptr) - (char*)currentMeta - sizeof(metadata) - currentMeta->size);
		int sizeNeeded = (numOfBytes + sizeof(metadata));
		if(sizeBtwn >= sizeNeeded && (currentMeta->ptr != NULL)){
			previousMeta = currentMeta;
			currentMeta = (metadata*)((char*)previousMeta + sizeof(metadata) + previousMeta->size);
			previousMeta->ptr = (char*)currentMeta;
			break;
		}
		else{
			if((char*)currentMeta->ptr < (char*)header || (char*)currentMeta->ptr > (char*)((char*)header + MEMORYSIZE)){
				currentMeta->ptr = NULL;
			}	
		
			if(currentMeta->ptr == NULL){
				if((metadata*)((char*)currentMeta-sizeof(metadata)) == (metadata*)header){
					header->ptr = (char*)currentMeta;
					currentMeta->ptr =  (char*)((char*)currentMeta+sizeof(metadata)+numOfBytes);
					currentMeta->free = false;			
					previousMeta->ptr = (char*)currentMeta;
					char* userpointer = (char*)((char*)currentMeta+sizeof(metadata));
					return userpointer;
					break;
				}


				break;
			}
				previousMeta = currentMeta;
				currentMeta = (metadata*)(currentMeta->ptr);
		}
	}
			
	currentMeta->size = numOfBytes;
	currentMeta->ptr = (char*)((char*)currentMeta+sizeof(metadata)+numOfBytes);
	currentMeta->free = false;
	
	
	char* userPointer = (char*)currentMeta+sizeof(metadata);
	return userPointer;
	
}

void myfree(char* userPointer, char* myfile, int myline){
	if(userPointer == NULL){
		printf("Null pointer given. File: %s, Line %d.\n", myfile, myline);
	}
	if(header==NULL){
		printf("Not dynamically allocated. File: %s, Line %d.\n", myfile, myline);
		return;
	}
	if((char*)userPointer < (char*)header || (char*)userPointer > (char*)header + MEMORYSIZE){
		printf("Pointer not allocated by malloc.  File: %s, Line: %d\n", myfile, myline);
		return;
	} 
	currentMeta = header; 
	//traverse the array and find metadata that links the userPointer.	
	while( (char*)currentMeta + (sizeof(metadata)) != (char *)userPointer && currentMeta->ptr != NULL){		
		previousMeta = currentMeta;
		currentMeta = (metadata*)currentMeta->ptr; 		
		if(currentMeta->ptr < (char*)header || currentMeta->ptr > (char*)header + MEMORYSIZE){
			currentMeta->ptr = NULL;
		}
	}
	if (currentMeta->ptr == NULL) {
		printf("Pointer misaligned. File: %s, Line: %d\n", myfile, myline);
		return;
	}			

	//unlink
	metadata* nextMeta = (metadata*)currentMeta->ptr;
	
	if(nextMeta->ptr == NULL){
		previousMeta->ptr = (char*)(((char*)header) + sizeof(metadata));
	}
	else{
		previousMeta->ptr = currentMeta->ptr;
	}
	currentMeta->free = true;
	currentMeta->ptr = NULL;
//	printf("Freed successfully\n");
	return;
}

