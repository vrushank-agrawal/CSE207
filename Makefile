
LINK_TARGET = \
	client \
	server

OBJS = \
	client.o \
	server.o

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)

clean: 
	rm -f $(REBUILDABLES)

client : client.o
	cc -g3 -o  $@ $< -lpthread

server : server.o
	cc -g3 -o  $@ $< -lpthread

%.o : %.c
	cc -g3 -Wall -o $@ -c $<

server.o: server.h

client.o: client.h