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
    packet_t packet;
    
    // to catch Ctrl-C
    struct sigaction act;
    act.sa_handler = intHandler;
	sigset_t set; 
	sigemptyset(&set);                                                             
	sigaddset(&set, SIGINT); 
	act.sa_mask = set;
	act.sa_flags = SA_NOCLDSTOP;   // just to remove valgrind error
    sigaction(SIGINT, &act, NULL);
    
    // creat boss FIFO file. if FIFO file already exits - delete it
    unlink(BOSS_FIFO);
    umask(0);
    if (mkfifo(BOSS_FIFO, 0777) < 0) {
        error_exit("mkfifo");
    }
    printf("BOSS: waiting for connections. Hit <Ctrl-c> to terminate the process.\n");
    
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

    for (;;) {
        
        // reading from boss FIFO
        if (read(boss_fifo, &packet, sizeof(packet)) <= 0) { 
            fprintf(stderr, "BOSS: error while reading from FIFO");
            continue;
        }
        
        switch (packet.code) {
            case CONNECT:
                printf("BOSS: worker(%d) is connected.\n", packet.pid);
                // full path to worker FIFO file
                char worker_fifo_path[WORKER_FIFO_NAME_LEN]= {0};  
                snprintf(worker_fifo_path, WORKER_FIFO_NAME_LEN, WORKER_FIFO_TEMPLATE, packet.pid);
                // open worker FIFO (outgoing)
                int worker_fifo = open(worker_fifo_path, O_WRONLY);
                if (worker_fifo < 0) {
                    fprintf(stderr, "Failed to open worker(%d) FIFO", packet.pid);
                    continue;
                }
                // send some work to the worker
                for (int i = 0; i < 26; i++) {
                    packet.data[i] = 'A' + i;
                }
                packet.data[26] = '\0';
                if (write(worker_fifo, &packet, sizeof(packet)) < 0) {
                    fprintf(stderr, "Failed to write to worker(%d) FIFO", packet.pid);
                }
                // close worker FIFO
                if (close(worker_fifo) < 0) {
                    fprintf(stderr, "Failed to close worker(%d) FIFO", packet.pid);
                }
                continue;
            case DISCONNECT:
                printf("BOSS: worker(%d) completed the job.\n", packet.pid);
                printf("BOSS: worker(%d) data : %s\n", packet.pid, packet.data);
                continue;
            default:
                fprintf(stderr, "BOSS: unknown message from a worker.\n");
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
