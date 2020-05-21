/*
* gcc -o boss -pedantic -std=gnu99 -Wall -Werror -ggdb3 boss.c
* gcc -o boss -pedantic -std=gnu99 -Wall -Werror boss.c && ./boss
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>      // errno
#include <unistd.h>     // fork() read()
#include <sys/wait.h>   // wait()
#include <sys/types.h>  // open() fork() wait() umask()
#include <sys/stat.h>   // open() umask()
#include <fcntl.h>      // open() close()
#include <string.h>     // memset()
#include <signal.h>
#include "globals.h"

/* prints error message and terminates the program */
void error_exit(char*);

/* signal handler for Ctrl+C */
void intHandler(int);

int dummy_fifo = 0;
int boss_fifo = 0;

int main(int argc, char** argv)
{
    char buffer[BUFSIZE] = { 0 };
	
	// to catch Ctrl+C
	struct sigaction act;
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, NULL);
	
	// creat boss FIFO file. if FIFO file already exits - delete it
    unlink(BOSS_FIFO);
    umask(0);
    if (mkfifo(BOSS_FIFO, 0777) < 0) {
        error_exit("mkfifo");
    }
    printf("BOSS: waiting for connections.\n");
	
    // open boss FIFO for reading
    boss_fifo = open(BOSS_FIFO, O_RDONLY);
    if (boss_fifo < 0) {
        error_exit("Failed to open boss FIFO");
    }
    printf("BOSS: FIFO is opened.\n");
	
	// open boss FIFO for writing to prevent EOF
    dummy_fifo = open(BOSS_FIFO, O_WRONLY);
    if (dummy_fifo < 0) {
        error_exit("Failed to open dummy FIFO");
    }

	while(1) {
		
		// reading from boss FIFO
		memset(buffer, '\0', BUFSIZE);
		if (read(boss_fifo, buffer, BUFSIZE) <= 0) { 
			fprintf(stderr, "BOSS: error while reading from FIFO");
			continue;
		}
		
		int worker_pid = 0;
		if (sscanf(buffer, "Done(%d)", &worker_pid) == 1) { // worker is done
			// here we are supposed to process the data from the worker
			printf("BOSS: worker(%d) is done.\n", worker_pid);
			continue;
		} else if (sscanf(buffer, "Connect(%d)", &worker_pid) == 1) { // worker just connected
			printf("BOSS: worker(%d) connected.\n", worker_pid);
			
			// full path to worker FIFO file
			char worker_fifo_path[WORKER_FIFO_NAME_LEN]= {0};  
			snprintf(worker_fifo_path, WORKER_FIFO_NAME_LEN, WORKER_FIFO_TEMPLATE, worker_pid);
			// open worker FIFO (outgoing)
			int worker_fifo = open(worker_fifo_path, O_WRONLY);
			if (worker_fifo < 0) {
				fprintf(stderr, "Failed to open worker(%d) FIFO", worker_pid);
				continue;
			}
			// send some data to the worker
			memset(buffer, '\0', BUFSIZE);
			for (int i = 0; i < (BUFSIZE - 1); i++) {
				buffer[i] = 48 + (i % 10);
			}
			if (write(worker_fifo, buffer, BUFSIZE) < 0) {
				fprintf(stderr, "Failed to write to worker(%d) FIFO", worker_pid);
			}
			// close worker FIFO
			if (close(worker_fifo) < 0) {
				fprintf(stderr, "Failed to close worker(%d) FIFO", worker_pid);
			}
		} else {
			fprintf(stderr, "BOSS: unknown message fron a worker: %s\n", buffer);
			continue;
		}
	}
   
    exit(EXIT_SUCCESS);
}

/* prints error message and terminates the program */
void error_exit(char* msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/* signal handler for Ctrl+C */
void intHandler(int i) {
    
	if (close(dummy_fifo) < 0) {
		error_exit("Failed to close dummy FIFO");
	} 
	if (close(boss_fifo) < 0) {
		error_exit("Failed to close boss FIFO");
	} 
	printf("BOSS: all FIFO are closed.\n");
	
	// delete FIFO file
    if (unlink(BOSS_FIFO) < 0) {
        error_exit("Failed to delete FIFO file");
    }
	exit(EXIT_SUCCESS);
}
