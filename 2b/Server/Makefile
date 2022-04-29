LINK_TARGET = server fancyclient client # variable

OBJS = server.o fancyclient.o client.o# variable

REBUILDABLES = $(OBJS) $(LINK_TARGET) # variable

all: $(LINK_TARGET)
# command that builds the LINK_TARGET after its dependencies

clean:
	rm -f $(REBUILDABLES)
# command to remove the REBUILDABLES which are the object files

server: server.o
	cc -g -o $@ $<

fancyclient: fancyclient.o
	cc -g -o $@ $<

client: client.o
	cc -g -o $@ $<

%.o: %.c
	cc -g -Wall -o $@ -c $<
# Command to build object files given c files

server.o: server.h
# change object file if header files changes

fancyclient.o: fancyclient.h
# change object file if header files changes

client.o: client.h
# change object file if header files changes