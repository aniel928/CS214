all: netfileserver.c netclient.c libnetfiles.o
	gcc -lm netclient.c -o netclient libnetfiles.o
	gcc -lpthread -lm netfileserver.c -o netfileserver
	
libnetfiles.o: libnetfiles.c libnetfiles.h
	gcc -c libnetfiles.c	

server: netfileserver.c
	gcc -lpthread -lm netfileserver.c -o netfileserver
	
client: netclient.c libnetfiles.o
	gcc -lm netclient.c -o netclient libnetfiles.o
	
clean: 
	rm *.o ; rm netfileserver ; rm netclient
