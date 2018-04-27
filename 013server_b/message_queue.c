#include "../exp1.h"
#include "message_queue.h"

// next message array pos
int msg_pos = 0;

// message queue
MessageQueue m_queue[MAX_QUEUE];

void push_msg_queue(char to[], char msg[]) {
    strncpy(m_queue[msg_pos].to, to, sizeof(m_queue[msg_pos].to));
    strncpy(m_queue[msg_pos].msg, msg, sizeof(m_queue[msg_pos].msg));

    msg_pos++;
    if(msg_pos >= MAX_QUEUE-1)
        msg_pos = 0;
}

// search msg queue
int search_msg_queue(char to[], char msg[]) {
    int i;
    for(i = 0; i < MAX_QUEUE; i++) {
        int idx = (msg_pos + i < 10)? msg_pos + i : msg_pos + i - 10;
        if(strcmp(m_queue[idx].to, to) == 0) {
            strncpy(msg, m_queue[idx].msg, sizeof(m_queue[idx].msg));
            memset(m_queue[idx].to, 0, sizeof(m_queue[0].to));
            memset(m_queue[idx].msg, 0, sizeof(m_queue[0].msg));
            return 1;
        }
    }

    return 0;
}