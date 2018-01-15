#ifndef _MYMALLOC_H
#define _MYMALLOC_H
#define malloc( x ) mymalloc( x , __FILE__, __LINE__ )
#define free( x ) myfree( x , __FILE__, __LINE__ )
#define MEMORYSIZE 5000
#define TESTLOOP 100
#endif

typedef enum _bool{false=0, true=1} bool;

typedef struct _metadata{
	char* ptr;
	bool free;
	int size;
} metadata;


char * mymalloc(int numOfBytes, char * myfile, int myline);
void myfree(char* userPointer, char* myfile, int myline);
