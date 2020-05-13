/*
* gcc -o worker -pedantic -std=gnu99 -Wall -Werror -ggdb3 worker.c
* gcc -o worker -pedantic -std=gnu99 -Wall -Werror worker.c && ./worker
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strlen() memset()
#include <unistd.h>
#include <sys/types.h>  // open()
#include <sys/stat.h>   // open()
#include <fcntl.h>      // open() close()

int main(int argc, char** argv)
{
    pid_t pid = getpid();
    int fd;
    
    if (argc != 3) {
        fprintf(stderr, "----Worker(%d): wrong number of arguments\n", pid);
        exit(EXIT_FAILURE);
    }
    int sleep_time = 0;
    if (!sscanf(argv[1], "%d", &sleep_time) || (sleep_time < 0)) {
        fprintf(stderr, "----Worker(%d): argument is wrong.\n", pid);
        exit(EXIT_FAILURE);
    }
    
    /* open the named pipe for writing*/
    fd = open(argv[2], O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    /* write something into the named pipe */
    char str[80] = { 0 };
    sprintf(str, "Worker(%d): going to do smth for %s sec.\n", pid, argv[1]);
    write(fd, str, strlen(str) + 1); 

    sleep(sleep_time);

    memset(str, '\0', 80);
    sprintf(str, "Worker(%d): terminating.\n", pid);
    write(fd, str, strlen(str) + 1); 
    
    /* close the named pipe */
    close(fd); 

    exit(EXIT_SUCCESS);
}
