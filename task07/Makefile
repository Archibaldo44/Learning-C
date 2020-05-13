.PHONY: clean all
.DEFAULT_GOAL: all
CFLAGS=-ggdb3 -Wall -Werror -pedantic -std=gnu99

all: boss worker

boss: boss.c
	gcc -o boss $(CFLAGS) boss.c

worker: worker.c
	gcc -o worker $(CFLAGS) worker.c

# just in case
clean:
	rm *.o boss worker
  
