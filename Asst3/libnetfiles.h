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



#define PORTNO 25432

int netopen(const char *pathname, int flags);
ssize_t netread(int filedes, void* buf, size_t nbyte);
ssize_t netwrite (int filedes, const void* buf, size_t nbyte);
int netclose (int fd);
int netserverinit(char * hostname);
int getSocket(struct sockaddr_in serverAI);

