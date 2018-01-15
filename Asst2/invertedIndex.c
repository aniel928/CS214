#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "invertedIndex.h"
#include <string.h>
#include <libgen.h>
	
words * headword = NULL;
int numDir = 0;
int numFile = 0;
int numWords = 0;


//compare strings, 0 is equal, -1 means string 1 is "less than" string 2, 1 means string 1 is "greater than" string 2
int stringCompare(char* string1, char* string2){
	int strSize = 0;
	int iter = 0;
	int result;
	if (strlen(string1)>strlen(string2)){			//if the first string has more characters than the second then the result is 1
		strSize = strlen(string2);			//used as a default if both of the strings have the same letters up until the strSize
		result = 1;
	}
	else if(strlen(string1)<strlen(string2)){		//if the first string has less characters than the second set the result to -1
		strSize = strlen(string1);			//this is done because if both strings have the same letters up to the first strings length
		result = -1;						//the first string is smaller than the second and therefore return -1
	}
	else{
		strSize = strlen(string1);
		result = 0;
	}
	while (iter < strSize){
		if (isalpha(string1[iter]) && isalpha(string2[iter])){		//if current characters being compared are both letters
			if (string1[iter] != string2[iter]){			//if they are not equal to eachother
				if (string1[iter] < string2[iter]){		//result is -1 if the first is smaller than the second	(ascii values)
					result = -1;	
					break;
				}
				else{						//result is 1 if the first is greater than the second	(ascii values
					result = 1;
					break;				
				}
			}
		}
		else if(isalpha(string1[iter]) && isdigit(string2[iter])){	//if characters being compared are a string and a digit the digit is greater
			result = -1;
			break;
		}
		else if (isdigit(string1[iter]) && isalpha(string2[iter])){
			result = 1;
			break;
		}
		else{ 
			if (string1[iter] != string2[iter]){			//if the current characters are not equal to eachother compare their ascii values
				if (string1[iter] < string2[iter]){		//return -1 if the first is less than the second, 1 if it is greater
					result = -1;
					break;
				}
				else{
					result = 1;
					break;				
				}
			}
		}
	iter++;									//increment iterator to access next character in string
	}
	return result;								//return the 
}


void printTable(words * firstWord, const char* fPath){
	files* fi = NULL;
	int fCheck = open(fPath, O_CREAT|O_RDWR, S_IRWXU);			//Opening the file
	write(fCheck, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", 39);
	write(fCheck, "<fileIndex>\n", 12);
	if(!firstWord){
		write(fCheck, "</fileIndex>\n",13);			
		close(fCheck);
		return;
	}
	fi = firstWord->fileList;					//file to iterate each words file list
	while (firstWord && fi){							//first word in list
		write(fCheck, "\t<word text=\"",13);
		write(fCheck, firstWord->word, strlen(firstWord->word));
		write(fCheck, "\">\n", 3);
		fi = firstWord->fileList;					//sets file to first file
		files* checkFile = fi;
		int maxCount = 0;
		while(checkFile != NULL){					//check if 
			if (maxCount < checkFile->wordCtr){
				maxCount = checkFile->wordCtr;
			}
			checkFile = checkFile->next;		
		}
		while(maxCount > 0){	
				checkFile = fi;
				while(checkFile!=NULL){					//printing the strings in numerical order
					if(checkFile->wordCtr == maxCount){						//checks if the count for the file count is equal to the max count
						write(fCheck, "\t\t<file name=\"",14);						
						write(fCheck, checkFile->fileName, strlen(checkFile->fileName));		//writes the string into the file
						write(fCheck, "\">", 2);						
						char* currStr = (char*)calloc(1,sizeof(int));
						if (currStr == NULL){
							printf("Calloc failed. Exiting Program.");
							exit(-1);
						}
						sprintf(currStr,"%d",checkFile->wordCtr);
						write(fCheck, currStr,strlen(currStr));		//writes the string into the file
						write(fCheck,"</file>\n",8);					
					}
					checkFile = checkFile->next;		//move to the next available
				}
				--maxCount;			//decrement max count to prepare to print files with smaller counts
		}	
		firstWord = firstWord->next;			
		write(fCheck, "\t</word>\n",9);
	}
	write(fCheck, "</fileIndex>\n",13);			
	close(fCheck);
	return;						//close file
}


//Insert word and file into table
void newWord(char* word, char *fName){
	numWords++;//increments for output
	files * currFile = NULL;
	files * prevFile = NULL;
	files * newFile = NULL;
	words * currWord = NULL;
	words * prevWord = NULL;
	words * newWord = NULL;
	
	
	if(headword == NULL){			//Used for creating headword
		headword = (words*)calloc(1,sizeof(words));		//allocate memory for headword, first word in list
		if(headword!=NULL){					//if headword is not null it needs to be created			
			headword->word = word;				
			headword->next = NULL;				
			newFile = (files*)calloc(1,sizeof(files));	//allocate size for first file node in file list for headword
			if(newFile == NULL){				
				printf("Calloc failed. Exiting Program.");
				exit(-1);
			}
			newFile->fileName = fName;			//give the file name to the new file node
			headword->fileList = newFile;			//set the file node to the file list in headword

			newFile->next = NULL;
			newFile->wordCtr = 1;
		}		
		else{	//calloc fails
			printf("Calloc failed. Exiting Program.");
			exit(-1);
		}
	}//End if headword null
	else{//headword not null					
		currWord = headword;					//Set currword to the begining of the word list
		int result = stringCompare(word, currWord->word);		//Iterate through the list to find where it goes using StringCompare 
		if(result == -1){
			//replace head
			newWord = (words*)calloc(1,sizeof(words));
			if(newWord == NULL){
				printf("Calloc failed. Exiting Program.");
				exit(-1);
			}
			newWord->word = word;
			newWord->next = currWord;
			headword = newWord;
			newFile = (files*)calloc(1,sizeof(files));
			if(newFile == NULL){
				printf("Calloc failed. Exiting Program.");
				exit(-1);
			}
			newWord->fileList = newFile;
			newFile->fileName = fName;
			newFile->next = NULL;
			newFile->wordCtr = 1;
		}//end result -1 - replacing head;
		else{	
			while(result > 0 && currWord != NULL){
				prevWord = currWord;
				currWord = currWord->next;
				if(currWord != NULL){
					result = stringCompare(word,currWord->word);
				}
			}//end while result < 0
			if(result == 0){//same word
				currFile = currWord->fileList;
				int fileResult = stringCompare(fName, currFile->fileName);
				if(fileResult == -1){
					newFile = (files*)calloc(1,sizeof(files));
					if(newFile == NULL){
						printf("Calloc failed. Exiting Program.");
						exit(-1);
					}
					newFile->fileName = fName;
					newFile->next = currFile;
					currWord->fileList = newFile;
					newFile->wordCtr = 1;
				}//end if fileResult = -1
				else{
					while(fileResult > 0 && currFile != NULL){
						prevFile = currFile;
						currFile = currFile->next;
						if(currFile != NULL){
							fileResult = stringCompare(fName, currFile->fileName);
						}					
					}//end while fileResult < 0
					if(fileResult == 0){//found same file
						(currFile->wordCtr)++;
					}//end fileResult == 0
					else{
						newFile = (files*)calloc(1,sizeof(files));
						if(newFile == NULL){
							printf("Calloc failed. Exiting Program.");
							exit(-1);
						}
						newFile->fileName = fName;
						newFile->next = currFile;
						prevFile->next = newFile;
						newFile->wordCtr = 1;
					}//end else (fileResult == 1)
				}//end else fileResult != -1
			}//end same word
			else{
				newWord = (words*)calloc(1,sizeof(words));
				if(newWord == NULL){
					printf("Calloc failed. Exiting Program.");
					exit(-1);
				}
				newWord->word = word;
				newWord->next = currWord;
				prevWord->next = newWord;
				newFile = (files*)calloc(1,sizeof(files));
				if(newFile == NULL){
					printf("Calloc failed. Exiting Program.");
					exit(-1);
				}
				newFile->fileName = fName;
				newFile->wordCtr = 1;
				newFile->next = NULL;
				newWord->fileList = newFile;			
			}//end else - result == 1
		}//end else (result != -1)
	}//end else headword not null	
}

void indexFile(char * pathname, char * currfile){
	numFile++;//increments for output
	int bufferRead = 0;  //pointer in buffer
	int fileRead = 0;	//pointer in file
	int readResult = 0; //number of bytes read so far
	int wordlength = 0; //word builder
	int countDown = 0; //to backtrack in buffer
	char* tempWord = 0; //to hold words across buffers
	int tempSet = 0; //to indicate that a tempword is set.

	//open file and report back error if not opened.	
	int currfd = open(pathname, O_RDONLY);//only need read access
	if(currfd == -1){
		fprintf(stderr,"Could not open file/directory due to: %s\n", strerror(errno));
		return;
	}
	//Find end of file (size)
	int filesize = lseek(currfd,0,SEEK_END);

	//reset file to first character
 	lseek(currfd,0,SEEK_SET);
 	
 	//declare buffer array
 	char buffer[BUFFER_SIZE] = {'\0'};
 	
 	readResult = read(currfd,buffer,BUFFER_SIZE);
 	
 	//If there's nothing in the file, exit.
 	if(readResult == 0){
 		return;
 	}
 	
 	//keeps track of current char in buffer
 	char nextchar = buffer[bufferRead];
 	
 	//before entering in the loop, get through any non-alpha (words can't start with numbers)
 	while(!isalpha(nextchar) && readResult <= filesize && fileRead <=readResult){
		bufferRead++;
 		fileRead++;
 		if(bufferRead == BUFFER_SIZE){//so we don't fall off end of buffer while waiting 
 			readResult += read(currfd,buffer,BUFFER_SIZE);
 			bufferRead = 0;
 		}
	 	nextchar = buffer[bufferRead];
 	}
	//While there's still file left to read, loop through and get all letters.
 	while(fileRead <= readResult){
		//While there's still buffer left to read, loop through and get all letters.
		while(bufferRead <= BUFFER_SIZE && fileRead <= filesize && readResult <=filesize){
			//if not alphanumeric, this will only trigger after a word is finished being set up.
			if(!isalpha(nextchar) && !isdigit(nextchar)){
				if(wordlength > 0){
					int i = 0;
					char* makeWord = (char*)calloc(1,wordlength+1);
					if(makeWord == NULL){
							printf("Calloc failed. Exiting Program.");
							exit(-1);
						}
					countDown = wordlength;
					for(i = 0; i < wordlength; i++){
						makeWord[i] = buffer[bufferRead - countDown];
						countDown--;
					}
					makeWord[wordlength] = '\0';
					//check to see if there's another word sitting from when we changed the buffer					
					if(tempSet == 1){
						int tempSize = strlen(tempWord) + strlen(makeWord) + 1;
						char * inBtwn = (char*)calloc(1,tempSize);
						if (inBtwn  == NULL){
							printf("Calloc failed. Exiting Program.");
							exit(-1);
						}
						strcat(inBtwn,tempWord);
						strcat(inBtwn,makeWord);
						strcpy(makeWord,inBtwn);
						char * makeWord = (char*)calloc(1,tempSize);
						if(makeWord == NULL){
							printf("Calloc failed. Exiting Program.");
							exit(-1);
						}
						strcpy(makeWord,inBtwn);
						tempSet = 0;
						wordlength = strlen(makeWord);
						free(inBtwn);
					}
					
					//change both file and words to lower case
					int n = 0;
					for(n = 0; n < strlen(currfile); n++){
						currfile[n] = tolower(currfile[n]);
					}
					for(n = 0; n < strlen(makeWord); n++){
						makeWord[n] = tolower(makeWord[n]);
					}			
					
					//second test to make sure we don't get any leading numbers (just to be safe)	
					if(isalpha(makeWord[0])){
						newWord(makeWord,currfile); //pass word and file nto hash table / linked lists
					}
					//reset word counter and incrmement all.
					wordlength = 0;
					bufferRead++;
					fileRead++;
					nextchar = buffer[bufferRead];
		}		//whle not alpha - prepare for next work, can't start with numbers
				while(!isalpha(nextchar) && (fileRead <= filesize) && readResult <= filesize && bufferRead <= BUFFER_SIZE){
					bufferRead++;
					fileRead++;
					nextchar = buffer[bufferRead];
					if(bufferRead == BUFFER_SIZE){//;protect from running off of end of buffer.
						readResult += read(currfd,buffer,BUFFER_SIZE);
						bufferRead = 0;
					}
				}
				
			}
			else{//if it hits here, it's part of a word, incremement wordlength and keep reading them in.
				wordlength++;
				bufferRead++;
		 		fileRead++;
				nextchar = buffer[bufferRead];
				//in case end of buffer is reached, this sets temp word.
				if(bufferRead == BUFFER_SIZE){
					tempWord = (char*)calloc(1,wordlength+1);
					if(tempWord  == NULL){
						printf("Calloc failed. Exiting Program.");
						exit(-1);
					}
					tempSet = 1;
					int i = 0;
					countDown = wordlength;
					for(i = 0; i < wordlength; i++){
						tempWord[i] = buffer[bufferRead - countDown];
						countDown--;
					}
					tempWord[wordlength] = '\0';
					wordlength = 0;
					bufferRead++;
				}
			}
 		} 		
 		readResult += read(currfd,buffer,BUFFER_SIZE);
		bufferRead = 0;
		nextchar = buffer[bufferRead];
 	}
 	//I don't know how necessary this is, but it's a catch all in case we come out of the loop without letters
 	//that didn't make it into the buffer.  Just a "play it safe" bit of code.  Same code as above.
	if(wordlength > 0){
		int i = 0;
		char* makeWord = (char*)calloc(1,wordlength+1);
		if (makeWord  == NULL){
			printf("Calloc failed. Exiting Program.");
			exit(-1);
		}
		countDown = wordlength;
		for(i = 0; i < wordlength; i++){
			makeWord[i] = buffer[bufferRead - countDown];
			countDown--;
		}
		makeWord[wordlength] = '\0';
		//check to see if there's another word sitting from when we changed the buffer					
		if(tempSet == 1){
			int tempSize = strlen(tempWord) + strlen(makeWord) + 1;
			char * inBtwn = (char*)calloc(1,tempSize);
			if(inBtwn == NULL){
				printf("Calloc failed.  Exiting Program.");
				exit(-1);
			}
			strcat(inBtwn,tempWord);
			strcat(inBtwn,makeWord);
			strcpy(makeWord,inBtwn);
			free(tempWord);
			free(makeWord);
			char * makeWord = (char*)calloc(1,tempSize);
			if(makeWord  == NULL){
				printf("Calloc failed. Exiting Program.");
				exit(-1);
			}
			strcpy(makeWord,inBtwn);
			tempSet = 0;
			wordlength = strlen(makeWord);
			free(inBtwn);
		}					
		//change both file and words to lower case
		int n = 0;
		for(n = 0; n < strlen(currfile); n++){
			currfile[n] = tolower(currfile[n]);
		}
		for(n = 0; n < strlen(makeWord); n++){
			makeWord[n] = tolower(makeWord[n]);
		}		
		//second test to make sure we don't get any leading numbers (just to be safe)			
		if(isalpha(makeWord[0])){
			newWord(makeWord,currfile);//pass word and file nto hash table / linked lists
		}
		wordlength = 0;
		bufferRead++;
		fileRead++;
		nextchar = buffer[bufferRead];
	}
	close(currfd);
	return;
}
	
void indexDirectory(char * pathname){
	int nameLen = 0;
	//open a dirent (directory entry -> file/directory/other type)
	struct dirent * nextFile = NULL;
	DIR * pathdir = opendir(pathname);
	//If not a directory, then it's a file -> happens when a file is passed in instead of a directory
	if(errno == ENOTDIR){
		char * base = (char*)calloc(1,strlen(pathname) - 2);
		if(base == NULL){
			printf("Calloc failed. Exiting Program.");
			exit(-1);
		}
		int i = 0;
		//get rid of ./ and use just the base since it's a file.
		while(i < strlen(pathname) - 2){
			base[i] = pathname[i+2];
			i++;
		}
		//since it's a file, go call that function.
		indexFile(pathname,base);
		free(base);
		return;
	}else{
		numDir++; //increments for output
	}
	int i = 0;
	do{
		if (pathdir == NULL){
		return;
		}
		nextFile = readdir(pathdir);
		if(nextFile == NULL){
			return;
		}
		//attach the next entry onto the end of the pathname and get ready to use it
		char * nextPath = (char*)calloc(1,strlen(pathname) + strlen(nextFile->d_name)+1 +i);
		if(nextPath == NULL){
			printf("Calloc failed. Exiting Program.");
			exit(-1);;
		}
		nameLen = strlen(pathname);
		strcpy(nextPath, pathname);
		nextPath[nameLen] = '/';
		strcat(nextPath, nextFile->d_name);
		
		//depending on the type of file do the following:
		if(nextFile != NULL){
			switch(nextFile->d_type){			//determine the file/directory type
				case DT_REG:
					indexFile(nextPath, nextFile->d_name); //index the file 
					break;
				case DT_DIR:
					//skip over '.' and '..' (first two entries of any directory.
					if((strcmp(nextFile->d_name,".\0") == 0) || (strcmp(nextFile->d_name, "..\0") == 0)){
						break;
					}
					//recursively call indexDirectory
					indexDirectory(nextPath);
					break;	
				//The rest of the cases can't be dealt with in our program and print to stderr			
				case DT_FIFO:
					fprintf(stderr,"Error, file type is a named pipe or FIFO\n");
					break;			
				case DT_SOCK:
					fprintf(stderr,"Error, file type is a local-domain socket.\n");
					break;			
				case DT_CHR:
					fprintf(stderr,"Error, file type is a character device.\n");
					break;			
				case DT_BLK:
					fprintf(stderr,"Error, file type is a block device.\n");
					break;			
				case DT_LNK:
					fprintf(stderr,"Error, file type is a symbolic link.\n");
					break;			
				case DT_UNKNOWN:
					fprintf(stderr,"Error, file type is unknown.\n");
					break;
				default:
					fprintf(stderr,"Error, file type is unknown.\n");			
			}//end switch
		}//end if
		i+=1;
	}while(nextFile != NULL);//end while
	free(nextFile);
	closedir(pathdir);
	return;
}//end index

int main(int argc, char ** argv){
	if(argc != 3){	//not enough/too many arguments given
		printf("Invalid number of arguments.  Please provide output file then file path.\n");
		exit(-1);
	}	
	
	//save output file and path info from user input.
	char * outfile = argv[1];
	char * fPath = (char*)calloc(1,strlen(outfile)+2);		
	if(fPath == NULL){
		printf("Calloc failed. Exiting Program.");
		exit(-1);
	}
	sprintf(fPath, "./%s", outfile);			//sets fPath to argv[1] file with its file path
	if( access (fPath, F_OK ) != -1){
		printf("The file '%s' already exists! Please pass in filename to create.\n",fPath);	//Checks to see if file exists, Not sure if it's working though	
		free(fPath);
		return -1;
	}
	char* pathname = argv[2];		//set pathname to last directory	
	if( access (pathname, F_OK ) == -1){
		printf("The path '%s' does not exist! Please pass in the name of an existing file or directory.\n",pathname);	//Checks to see if file exists, Not sure if it's working though	
		return -1;
	}
	
	indexDirectory(pathname);	//call indexDirectory and give it the path for the directory

		
	printTable(headword, fPath);	//call print table function, give it the first word in the hash table and the path for the outfile
	
	//Free the linked list of linked lists.
	words* currWord = headword;
	words* backWord;
	files* currFile;
	files* backFile;
	while (currWord){
		backWord = currWord;
		currFile = backWord->fileList;
		while(currFile){
			backFile = currFile;
			currFile = currFile->next;
			free(backFile);					
		}
	free(backWord);
	currWord = currWord->next;

	}
	//final output to user.
	printf("%d directories, %d files, and %d words indexed.  Output can be found at ./%s.\n",numDir,numFile,numWords,outfile);
	return 0;
}
