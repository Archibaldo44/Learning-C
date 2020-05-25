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

/* reverses a string */
void str_reverse(char*);

int main(void)
{
    // check if boss FIFO file exists
    if (access(BOSS_FIFO, F_OK) < 0 ) {
        error_exit("Boss FIFO file doesn't exist");
    }

    pid_t pid = getpid();                               // process pid
    char worker_fifo_path[WORKER_FIFO_NAME_LEN]= {0};  // to keep full path to FIFO file
    snprintf(worker_fifo_path, WORKER_FIFO_NAME_LEN, WORKER_FIFO_TEMPLATE, pid);
    
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

    // send CONNECT message to the boss
    packet_t packet = {.pid = pid, .code = CONNECT};
    if (write(boss_fifo, &packet, sizeof(packet)) < 0) {
        error_exit("Failed to write to boss FIFO");
    }

    // open worker FIFO
    int worker_fifo = open(worker_fifo_path, O_RDONLY);
    if (worker_fifo < 0) {
        error_exit("Failed to open worker FIFO");
    }

    // wait for a data from the boss (read from boss FIFO)
    if (read(worker_fifo, &packet, sizeof(packet)) < 0) { 
        // smth wrong happend; probably FIFO was closed
        error_exit("Failed to read from incoming FIFO");
    }
    
    // got data from the boss, doing some work
    printf("Worker(%d) received data from boss = '%s'\n", packet.pid, packet.data);
    str_reverse(packet.data);
    sleep(2);
    
    // send DISCONNECT message to the boss (including processed data)
    packet.pid  = pid;
    packet.code = DISCONNECT;
    if (write(boss_fifo, &packet, sizeof(packet)) < 0) {
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

/* reverses a string */
void str_reverse(char* string)
{
    int len = strlen(string);
    if ((len == 0) || (len == 1)) {  // empty string or just 1 letter
        return;                      // nothing to reverse    
    }
    
    char buf = '\0';
    char* first = string;           // pointer to the first symbol
    char* last  = string + (len - 1);  // pointer to the last symbol
    
    while (first < last) {
        buf = *first;
        *first = *last;
        *last = buf;
        first++;
        last--;
    }
    return;
}
