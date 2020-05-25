#define BOSS_FIFO "./boss_fifo"
#define WORKER_FIFO_TEMPLATE "./worker_%d_fifo"
#define WORKER_FIFO_NAME_LEN (sizeof(WORKER_FIFO_TEMPLATE) + 9)
#define BUFSIZE 27

/* структура пакета-запроса */
typedef struct packet {
    int  pid;                /* worker PID */
    int  code;               /* request code */
	char data[BUFSIZE];      /* */
} packet_t;

/* request codes (коды запросов) */
#define CONNECT         0       /* запрос на соединение */
#define DISCONNECT      1       /* разрыв связи */
#define WORK            2       /* data from the boss to a worker */
