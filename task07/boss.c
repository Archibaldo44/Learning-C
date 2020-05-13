/*
* gcc -o boss -pedantic -std=gnu99 -Wall -Werror -ggdb3 boss.c
* gcc -o boss -pedantic -std=gnu99 -Wall -Werror boss.c && ./boss
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>       // time()
#include <unistd.h>     // fork() read()
#include <sys/wait.h>   // wait()
#include <sys/types.h>  // open() fork() wait() umask()
#include <sys/stat.h>   // open() umask()
#include <fcntl.h>      // open() close()
#include <string.h>     // memset()
#define MIN_RANDOM 2
#define MAX_RANDOM 7
#define WORKER_NUMBER 5
#define NAMEDPIPE_NAME "/tmp/my_named_pipe"
#define BUFSIZE 80

/* returns a string which is a random integer from range [min,max] */
int get_random_range(void);

int main(void)
{
    char rand_str[4] = { 0 };  // helper - to convert int to string
    
    srand((unsigned)time(0));
    
    /* create named pipe */
    umask(0);
    if (mkfifo(NAMEDPIPE_NAME, 0777) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    
    /* starting WORKER_NUMBER workers */
    for (size_t i = 0; i < WORKER_NUMBER; i++) {
        sprintf(rand_str, "%d", get_random_range());  // convert rand int to string
        pid_t pid = fork(); 
        switch (pid) {
            case -1:
                perror("fork");
                break;
            case 0:    // child
                execl("worker", "", rand_str, NAMEDPIPE_NAME, NULL);
            default:    // parent
                break;
        }
    }
    printf("BOSS: working\n");
    
    /* open the named pipe for reading */
    printf("BOSS: trying to open the named pipe\n");
    int fd = open(NAMEDPIPE_NAME, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    /* read everithing from the named pipe */
    char string[BUFSIZE] = { 0 };
    while (read(fd, string, (BUFSIZE - 1)) > 0) {
        printf("from FIFO: %s", string);
        memset(string, '\0', BUFSIZE);
    }
    
    /* close the named pipe */
    if (close(fd) < 0) {
        perror("close");
        exit(EXIT_FAILURE);
    }
    printf("BOSS: the named pipe is closed\n");
    
    /* remove the named pipe for reading */
    if (remove(NAMEDPIPE_NAME) < 0) {
        perror("remove");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}

/* returns a string which is a random integer from range [min,max] */
int get_random_range(void)
{
    return ((rand() % (MAX_RANDOM - MIN_RANDOM + 1)) + MIN_RANDOM);
}
