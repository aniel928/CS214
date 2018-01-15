#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mymalloc.h"
#include <time.h>

void testA(){
	int n = 0;
	int c1 = 0;
	int c2 = 0;

	char* pointerArray[1000];
	while(n < 1000){
		pointerArray[n] = malloc(1);
		if(pointerArray[n]!=NULL){
			c1++;
		}
		n++;
	}
	n = 0;
	while(n < c1){
	//	printf("%p\n", pointerArray[n]);
		free(pointerArray[n]);
		n++;
		c2++;
	}
	//printf("%d, %d\n",c1, c2);	
}

void testB(){
//test B - malloc one, free one, 1000 times.
	int n = 0;
	n = 0;
	while (n < 1000){
		char * temp = malloc(1);
		free(temp);
		n++;
//		printf("%d, %p\n",n, temp);
	}
}
int randomnum(){
    
    int num = rand() % 2;
    return num;
}
int randomnum2(){
    
    int num = rand() % 64;
    return num;
}

void testC() {
//test C - random 1 byte
	time_t t;
	srand((unsigned)time(&t));
    char* pointerArray[1000];
    int coin=0;
    int i=0;
    while(i < 1000){
		pointerArray[i] = NULL;
		i++;
	}
    i=0;
  	int mallocGood=0;

  	while(mallocGood<1000){
    	
    	coin=randomnum();
    	if(coin==1){
            pointerArray[mallocGood]=(char*)malloc(1);
        	if(pointerArray[mallocGood]!=NULL){
            	mallocGood++;
         //   	printf("%d\n",mallocGood);
            	continue;
        	}
        	continue;
    	}
    	if(coin==0 && mallocGood>0){
    		int i;
			for(i=0; i < 1000 ;i++){
				if(pointerArray[i] != NULL){
					free(pointerArray[i]);
					pointerArray[i]=NULL;	
					continue;
				}					
			continue;
        	}
        continue;
		}
		if(mallocGood == 0){
			continue;
		}
    }
		for( i=0; i<1000; i++){
     		if(pointerArray[i]!=NULL){
     			free(pointerArray[i]);
     			i++;
 			}
      	}
}

void testD(){
//test D - random bigger byte
    time_t t;
	srand((unsigned)time(&t));
    char* pointerArray[1000];
    int coin=0;
    int i=0;
    int bytes = randomnum2();
    while(i < 1000){
		pointerArray[i] = NULL;
		i++;
	}
    i=0;
  	int mallocGood=0;

  	while(mallocGood<1000){
    	srand((unsigned)time(&t));
    	coin=randomnum();
    	if(coin==1){
            pointerArray[mallocGood]=(char*)malloc(bytes+1);
        	if(pointerArray[mallocGood]!=NULL){
            	mallocGood++;
            	printf("%d\n",mallocGood);
            	continue;
        	}
			if(pointerArray[mallocGood]==NULL){
	        	coin = 0;
	        }
    	}
    	if(coin==0 && mallocGood>0){
    		int i;
			for(i=0; i < 1000 ;i++){
				if(pointerArray[i] != NULL){
					free(pointerArray[i]);
					pointerArray[i]=NULL;	
					continue;
				}				
			continue;
        	}
        continue;
		}
		if(mallocGood == 0){
			continue;
		}
    }
		for( i=0; i<1000; i++){
     		if(pointerArray[i]!=NULL){
     			free(pointerArray[i]);
     			i++;
 			}
      	}
}
void testE(){
//test E - same random number, malloc and free repeatedly
	srand(time(NULL));
	int randNum = (rand()%8)+1;
	int i=0;
	char * temp;
	while(i < 100){
		temp = (char*)malloc(randNum);
		free(temp);
		i++;	
	}
	
}

void testF(){
//test F - different random numbers, malloc and free repeatedly.
	srand(time(NULL));
	int randNum;
	int i = 0;
	char * temp;
	for(i=0;i<100;i++){
		randNum = rand()%10;
		temp = (char*)malloc(randNum+1);
		free(temp);
	}	
	
}

int main(int argc, char** argv){
	clock_t start, end;

	
	
	int i = 0;
	start=clock();
	while (i < TESTLOOP){
		testA();
		i++;
	}
	end = clock();
	double A = (((double)(end-start)/CLOCKS_PER_SEC))/TESTLOOP;

	i = 0;
	start=clock();
	while (i < TESTLOOP){
		testB();
		i++;
	}
	end = clock();
	double B = (((double)(end-start)/CLOCKS_PER_SEC))/TESTLOOP;

	i = 0;
	start=clock();
	while (i < TESTLOOP){
		testC();
		i++;
	}
	end = clock();
	double C = (((double)(end-start)/CLOCKS_PER_SEC))/TESTLOOP;

	i = 0;
	start=clock();
	while (i < 1){
		testD();
		i++;
	}
	end = clock();
	double D = (((double)(end-start)/CLOCKS_PER_SEC))/TESTLOOP;

	i = 0;
	start=clock();
	while (i < TESTLOOP){
		testE();
		i++;
	}
	end = clock();
	double E = (((double)(end-start)/CLOCKS_PER_SEC))/TESTLOOP;

	i = 0;
	start=clock();
	while (i < TESTLOOP){
		testF();
		i++;
	}
	end = clock();
	double F = (((double)(end-start)/CLOCKS_PER_SEC))/TESTLOOP;


	printf("Average TestA = %f seconds \n", A);
	printf("Average TestB = %f seconds \n", B);
	printf("Average TestC = %f seconds \n", C);
	printf("Average TestD = %f seconds \n", D);
	printf("Average TestE = %f seconds \n", E);
	printf("Average TestF = %f seconds \n", F);

}