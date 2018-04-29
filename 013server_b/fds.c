#include "../exp1.h"
#include "fds.h"

// fds array
struct pollfd fds[MAX_CLIENTS];

// fds num
nfds_t nfds = 1;

// seconds of timeout
int timeout;

// return pollfd
struct pollfd get_fd(int i) {
    return fds[i];
}

// return nfds;
nfds_t get_nfds() {
    return nfds;
}

int start_poll() {
    return poll(fds, nfds, timeout);
}

void init_fds(int sock_listen) {
	memset(fds, 0 , sizeof(fds));
	fds[0].fd = sock_listen;
	// There is data to read.
	fds[0].events = POLLIN;
	// infinity!!!!!!!!
	timeout = -1;
	//timeout = (3 * 60 * 1000);
}

int add_fds(int sock_listen, struct sockaddr_in *client) {
	int       sock;
	socklen_t client_len = sizeof(client);

	sock = accept(sock_listen, (struct sockaddr *)client, &client_len);

	if (sock < 0) {
		if (errno != EWOULDBLOCK) {
			perror("\033[31m! accept() failed\033[39m");
			return -1;
		}
		return -1;
	}

	printf("\033[32m[accepted connection from %s:%d]\033[39m\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port));

	fds[nfds].fd = sock;
	fds[nfds].events = POLLIN;
	nfds++;

    return sock;
}

// close all fds.
void close_all_fds() {
	int i;
	for (i = 0; i < nfds; i++) {
		if(fds[i].fd >= 0)
			close(fds[i].fd);
	}
}

// close fd
void close_fd(int fd_num) {
	fds[fd_num].fd = -1;
}

// compress array
void compress_array() {
	int i, j;
	for (i = 0; i < nfds; i++) {
		if (fds[i].fd == -1) {
			for(j = i; j < nfds; j++) {
				fds[j].fd = fds[j+1].fd;
			}
			nfds--;
		}
	}
}