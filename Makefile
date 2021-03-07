SRC = $(wildcard *.c) 
HDR = $(wildcard *.h) 
COMPILE = gcc -W -o build/pool_alloc.o pool_alloc.c 

.PHONY: create
.PHONY: clean

create: ${SRC} ${HDR}
	${COMPILE} 

clean: # cleaning all output files for the project
	rm build/*.o