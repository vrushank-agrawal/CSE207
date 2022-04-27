
LINK_TARGET = \
	client

OBJS = \
	client.o

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)

clean: 
	rm -f $(REBUILDABLES)

client : client.o
	cc -g3 -o  $@ $< -lpthread

client.o : client.c
	cc -g3 -Wall -o $@ -c $<

client.o: client.h
