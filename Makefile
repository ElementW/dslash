#makefile

CC=gcc
CFLAGS=-pg -g -Wall
CPPFLAGS=
OBJS=  dslash.o
FINAL = dslash

all : dslash


debug: $(OBJS)
	$(CC) -g -Wall -DMEMWATCH -o $(FINAL) $(OBJS)


dslash : $(OBJS)
	$(CC) $(CFLAGS) -o $(FINAL) $(OBJS)
	
clean: 
	rm -f $(OBJS) $(FINAL) core

