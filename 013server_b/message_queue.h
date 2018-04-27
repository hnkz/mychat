#define MAX_QUEUE 10

typedef struct _message_queue {
  char to[32];
  char msg[1024];
} MessageQueue;

void push_msg_queue(char to[], char msg[]);
int  search_msg_queue(char to[], char *msg);