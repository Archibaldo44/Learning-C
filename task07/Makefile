.PHONY: clean all
.DEFAULT_GOAL: all
CFLAGS=-ggdb3 -Wall -Werror -pedantic -std=gnu99

all: boss worker

boss: boss.c globals.h
	gcc -o boss $(CFLAGS) boss.c

worker: worker.c globals.h
	gcc -o worker $(CFLAGS) worker.c

# just in case
clean:
	rm *.o boss worker ./boss_fifo ./worker_*
  
