/*
* gcc -o worker -pedantic -std=gnu99 -Wall -Werror -ggdb3 worker.c
* gcc -o worker -pedantic -std=gnu99 -Wall -Werror worker.c && ./worker
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>      // errno
#include <string.h>     // strlen()
#include <unistd.h>     // write()
#include <sys/types.h>  // open()
#include <sys/stat.h>   // open()
#include <fcntl.h>      // open() close()
#include "globals.h"

/* prints error message and terminates the program */
void error_exit(char*);

int main(void)
{
	// check if boss FIFO file exists
	if (access(BOSS_FIFO, F_OK) < 0 ) {
		error_exit("Boss FIFO file doesn't exist");
	}

	pid_t pid = getpid();                               // process pid
	char worker_fifo_path[WORKER_FIFO_NAME_LEN]= {0};  // to keep full path to FIFO file
	snprintf(worker_fifo_path, WORKER_FIFO_NAME_LEN, WORKER_FIFO_TEMPLATE, getpid());
	
	// create worker FIFO file, if file already exits - delete it
    unlink(worker_fifo_path);
	umask(0);
	if (mkfifo(worker_fifo_path, 0777) < 0) {
        error_exit("mkfifo");
    }

	// open BOSS_FIFO (outgoing)
    int boss_fifo = open(BOSS_FIFO, O_WRONLY);
    if (boss_fifo < 0) {
        error_exit("Failed to open outgoing FIFO");
    }

	// send pid to boss as "Connect(12345)"
	char buffer[BUFSIZE] = { 0 };
	snprintf(buffer, BUFSIZE, "Connect(%d)", pid);
	if (write(boss_fifo, buffer, BUFSIZE) < 0) {
		error_exit("Failed to write to boss FIFO");
	}

	// open worker FIFO
	int worker_fifo = open(worker_fifo_path, O_RDONLY);
	if (worker_fifo < 0) {
        error_exit("Failed to open worker FIFO");
    }

	// wait for smth from boss (read from boss FIFO)
	memset(buffer, '\0', BUFSIZE);
	if (read(worker_fifo, buffer, BUFSIZE) < 0) { 
		// smth wrong happend; probably FIFO was closed
		error_exit("Failed to read from incoming FIFO");
    }
	
	printf("Worker(%d) received data from boss = '%s'\n", pid, buffer);
	sleep(5);
	
	// send exit message to boss. "Done(12345)"
	memset(buffer, '\0', BUFSIZE);
	snprintf(buffer, BUFSIZE, "Done(%d)", pid);
	if (write(boss_fifo, buffer, BUFSIZE) < 0) {
		error_exit("write");
	}
	
	// close boss FIFO
	if (close(boss_fifo) < 0) {
		error_exit("Failed to close boss FIFO");
	}
	
	// close worker FIFO
	if (close(worker_fifo) < 0) {
		error_exit("Failed to close worker FIFO");
	} 
	
	// delete FIFO file
	if (unlink(worker_fifo_path) < 0) {
        error_exit("Failed to delete worker FIFO file");
    }

    exit(EXIT_SUCCESS);
}

/* prints error message and terminates the program */
void error_exit(char* msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}
