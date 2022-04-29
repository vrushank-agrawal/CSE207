LINK_TARGET = client # variable

OBJS = client.o # variable

REBUILDABLES = $(OBJS) $(LINK_TARGET) # variable

all: $(LINK_TARGET)
# command that builds the LINK_TARGET after its dependencies

clean:
	rm -f $(REBUILDABLES)
# command to remove the REBUILDABLES which are the object files

$(LINK_TARGET): $(OBJS)
	cc -g -o $@ $^
# Command to build the target and link with the object files

%.o: %.c
	cc -g -Wall -o $@ -c $<
# Command to build object files given c files

client.o: client.h
# change object file if header files changes