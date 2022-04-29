LANGUAGE=HINDI

# all
all: hello-world international-hello-world fancy-hello-world

# hello-world
hello-world: hello-world.o
	cc -g -o hello-world hello-world.o

hello-world.o: hello-world.c
	cc -c -Wall -g hello-world.c

# international-hello-world
international-hello-world: international-hello-world.o
	cc -g -o international-hello-world international-hello-world.o

international-hello-world.o: international-hello-world.c
	cc -c -Wall -g international-hello-world.c -D$(LANGUAGE)

# fancy-hello-world
fancy-hello-world: fancy-hello-world.o
	cc -g -o fancy-hello-world fancy-hello-world.o
 
fancy-hello-world.o: fancy-hello-world.c
	cc -c -Wall -g fancy-hello-world.c

# clean commands
clean:
	rm hello-world.o international-hello-world.o fancy-hello-world.o hello-world international-hello-world fancy-hello-world

clean-hello-world:
	rm hello-world.o hello-world

clean-international-hello-world:
	rm international-hello-world.o international-hello-world

clean-fancy-hello-world:
	rm fancy-hello-world.o fancy-hello-world