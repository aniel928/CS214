#include <stdio.h>
//#ifndef
#define BUFFER_SIZE 100
//#endif


typedef struct _files{
	char * fileName;
	struct _files * next;
	int wordCtr;
} files;
typedef struct _words{
	char * word;
	struct _words * next;
	files * fileList;
} words;



void indexFile(char * pathname, char * currfile);

void indexDirectory(char * pathname);

void newWord(char* word, char *fName);

void printTable(words * firstWord, const char* fPath);

int stringCompare(char* string1, char* string2);