#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct _node {
	char* wordptr;//pointer to word
	struct _node *link;//pointer to next linked node.
}node;

//initiate global variables and nodes
node *head=NULL;
node *current=NULL;
node *newNode=NULL;
int nodeCounter = 0;



node* makeLinkedList(char * inputString, int sizeOfInput){
	int i = 0;
	int k = 0;
	char* word;
	int wordLength=0;
	char currLetter;
	
	/*loop through all of input without "falling off end" - takes current letter in loop and goes one   *
	 *character at a time, then checks to see if current Letter is alphabetic or not.    				*/
	while(i <= sizeOfInput){
		currLetter = *(inputString+i);
		if(isalpha(currLetter)){	
			//valid letter - this is part of a word - increment current word length
			wordLength++;
		}
		else{
			/*if input starts with a delimiter, or if multiple delimiters occur one after another, move	*
			 *forward to next character, this prevents a new line being printed when no words are given	*/
			if(wordLength==0){
				i++;
				k++;
				continue;
			}
			//not a valid letter, time to append to node and move to next
			word = (char*)malloc((wordLength)*sizeof(char));//create a char* word
			if (word==NULL){//if memory doesn't allocate
				printf("Memory could not be allocated, please try again.");
				return(NULL);
			}
			else{
				//fill in new word
				int j=0;
				while(j <= wordLength){
					if(j == wordLength){
						word[j]='\0';
					}
					else{
						word[j]=inputString[j+k];//j+k keep us to the proper spot in the input string
					}
					j++;
				}
			}
			k+=(wordLength+1);//keeps track of where we are in input string
			wordLength=0; //reset for new word

			// NODE PART:
			newNode = (node*)malloc(sizeof(node));
			if(newNode == NULL){//if malloc fails
				printf("Memory could not be allocated, please try again.");
				return(NULL);
			}
			
			//append to list
			newNode->wordptr = word;
			newNode->link = NULL;

			if(head==NULL){//if head is null, then we need to create the head
				head = newNode;
				nodeCounter++;//keep track of # of nodes total
			}
			else{//else find next empty link and link new node in.
				nodeCounter++;
				current = head;
				while(current->link != NULL){
					current = current->link;
				}
				current->link = newNode;
			}
		}


		i++;
	}
	return(head);
}

//take linked list and put into an array
char ** linkedListToArray(node *head, int length){
	node * ptr = head;
	int num = 0;
	char ** arrayOfWords = (char**)malloc(sizeof(char*)*length);
	if(arrayOfWords == NULL){
			printf("Memory could not be allocated, please try again.");
			return(NULL);
		}
		while(num<length){
	    arrayOfWords[num]= ptr->wordptr;  
    	ptr=ptr->link;
    	num++;
	}
	return arrayOfWords;
}

//sort the array into ASCII alphabetical order
char ** iSortArrays(char ** charArray, int length){
	int arrayLength=length-1;//arrays start from 0, so need 1 - length
	int i=0;
	int k=0;

	//compare the two strings 
	for(i=0; i<= arrayLength; i++){
    	int index =i;
    	for(k=i+1; k<= arrayLength; k++){
    		int compare = strcmp(charArray[index], charArray[k]);
    		if (compare>0){
    		    index=k;
    		}
    	}
    	
		//sort swap if necessary
    	if(index!=i){
    	    char * copy = charArray[i];
    	    charArray[i]=charArray[index];
    	    charArray[index]=copy; 
    	}
	}
	return charArray;
}

//print the sorted array
void printArray(char ** wordArr, int listLength) {
	int i;
    	for(i = 0; i <= listLength-1; i++) {
	        	printf("%s\n", wordArr[i]);
    	}
}

int main(int argc, char *argv[]){
	//check to make sure you only get one argument
	char* userInput;
	int sizeInput=0;
	if(argc !=2){
		printf("Incorrect number of parameters.\n");
		return(1);
	}

	userInput = argv[1];//assign input to char* variable
	sizeInput = strlen(userInput);//get size of input so we don't "fall off end"
	
	//read input string and make linked list
	head = makeLinkedList(userInput,sizeInput);
	if(head==NULL){
		return(1);
		}

	//take linked list and put into an array
	char ** arrayOfNodes = linkedListToArray(head, nodeCounter);
	if(arrayOfNodes == NULL){
		return(1);
	}
	
	//take array and sort it
	char ** sortedArray = iSortArrays(arrayOfNodes, nodeCounter);

	//take array and print it
	printArray(sortedArray, nodeCounter);
	
	
	//Free everything!
	free(arrayOfNodes);	
	node * previous = NULL;
	current = head;
	
	while(head->link!=NULL){
		while(current->link!=NULL){
			previous = current;//store second to last node so we can get back to it
			current = current->link;//now go to last
		}			
		free(current->wordptr);//free last word
		current = previous;
		free(current->link);//free last node
		current->link=NULL;
		current=head;

	}
		//free head word/node
		free(head->wordptr);
		free(head);
		
	return(0);
}
