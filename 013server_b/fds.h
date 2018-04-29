#include <poll.h>

#define MAX_CLIENTS 200

int start_poll();
struct pollfd get_fd(int i);
nfds_t get_nfds();
void init_fds(int sock_listen);
int add_fds(int sock_listen, struct sockaddr_in *client);
void close_all_fds();
void close_fd(int fd_num);
void compress_array();