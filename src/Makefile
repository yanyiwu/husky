INCLUDE= \
	 -I../util 

#CFLAG = -c -g -pg -Wall -fPIC 
CFLAG = -O2 -pipe 


GCC = g++
CC = gcc
.SUFFIXES: .o .cpp .c

OBJS = Daemon.o\
	FileOperation.o\
	XmlHttp.o\
	ServerFrameLockAccept.o#\
	#Dic.o\
	InvertReader.o\
	StringTrim.o\
	TaDicW.o\
	BitMapFile.o\
	MMapFile.o\

.INTERMEDIATE: $(OBJS)
all:  libutil.a 

Server: main.cpp Server.cpp
	$(GCC) $(CFLAG) -o $@ $^ libutil.a -lpthread

libutil.a: $(OBJS)
	 ar -crv $@ $(OBJS);

.cpp.o:
	$(GCC) -c $(CFLAG) -o $@ $< $(INCLUDE) -D_FILE_OFFSET_BITS=64  -DUSEBDB

.c.o:
	$(GCC) -c $(CFLAG) -o $@ $< $(INCLUDE) -D_FILE_OFFSET_BITS=64  -DUSEBDB

clean:
	rm -f *.o
	rm -f libutil.a

