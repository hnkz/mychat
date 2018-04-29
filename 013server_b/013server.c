#include "../exp1.h"
#include "../exp1lib.h"
#include <poll.h>
#include "db.h"
#include "fds.h"
#include "message_queue.h"

typedef struct _user {
	char  client_name[128];
	char  ip_port[128];
	int   login_flag;
} User;

// flag of compressing array
int compress_array_flag;

// flag of ending server
int end_server = 0;

// user
User user[MAX_CLIENTS];

void  	init_clients(int sock_listen);
void  	shut_client(int fd_num);
void  	process_command(int fd_num, char command[]);
void  	send_to(char user[], char msg[]);
void  	broadcast_clients(int fd_num, char buf[]);
int   	recv_clients(int fd_num, char msg[]);
void  	do_server(int sock_listen);
void  	check_fds(int sock_listen, int max_fds_num);
int   	is_recv_error(int len, int fd_num);
void  	login(int fd_num);
void 	send_queue(int fd_num, char to[]);

// init pollfd and so on.
void init_clients(int sock_listen) {
	int ret, on = 1;

	ret = ioctl(sock_listen, FIONBIO, (char *)&on);
	if(ret < 0) {
		perror("ioctl() failed");
		close(sock_listen);
		exit(-1);
	}

	init_fds(sock_listen);

	// init user struct
	memset(user, 0, sizeof(user));
}

// shut client
void shut_client(int fd_num) {
	char log_val[256];
	struct pollfd fd = get_fd(fd_num);
	printf("\033[31m! Close clients(fd = %d)\033[39m\n", fd.fd);

	// log
	snprintf(log_val, sizeof(log_val), "'%s', '%s', current_timestamp", user[fd.fd].client_name, user[fd.fd].ip_port);
	insert_log(log_val, "logout_log");
	memset(&user[fd.fd], 0, sizeof(user[fd.fd]));
	close_fd(fd_num);
	close(fd.fd);
	compress_array_flag = 1;
}

// If client send command, process it.
void process_command(int fd_num, char command[]) {
	static char command_list[][32] = {
		"\\dm ", // direct message
		"\\log ", // print my message log
	};
	char *tmp = NULL;
	int command_num;
	int command_size = sizeof(command_list)/sizeof(command_list[0]);
	struct pollfd fd = get_fd(fd_num);

	int i;
	for(i = 0; i < command_size; i++){
		tmp = strstr(command, command_list[i]);
		if(tmp != NULL) {
			tmp = &tmp[strlen(command_list[i])];
			command_num = i;
			break;
		}
	}

	switch(command_num) {
		// dm
		case 0:
		;
			char *start_p, *end_p;
			char username[128], msg[1024];
			int start, end, len;

			memset(username, 0, sizeof(username));
			memset(msg, 0, sizeof(msg));

			start_p = strchr(tmp, '<');
			end_p   = strchr(tmp, '>');

			if(start_p == NULL || end_p == NULL) {
				printf("\033[31m! error command\033[39m\n");        
				break;
			}

			start = start_p - tmp + 1;
			end   = end_p - tmp;
			len = end - start;
			
			strncpy(username, start_p+1, len);
			snprintf(msg, sizeof(msg), "(%s): %s", user[fd.fd].client_name, end_p+2);

			send_to(username, msg);

			break;
		// log
		case 1:
		;

			break;
		default:
			break;
	}
}

// do server loop
void do_server(int sock_listen) {
	int i;
	int ret;
	int current_fds_size;
	char msg[1024];
	struct pollfd fd;

	do {
		ret = start_poll();
		
		if (ret < 0) {
			perror("\033[31mpoll() failed\033[39m");
			break;
		} else if (ret == 0) {
			perror("\033[31mpoll() timed out.  End program.\033[39m\n");
			break;
		}

		/*
		 * Each fd has each process.
		 *  sock_listen -> accept
		 *  client -> read, broadcast, login
		 */
		current_fds_size = get_nfds();
		for (i = 0; i < current_fds_size; i++) {
			fd = get_fd(i);
			if(fd.revents == 0)
				continue;
			if(fd.revents != POLLIN) {
				printf("\033[31m! Error! revents = %d\033[39m\n", fd.revents);
				end_server = 1;
				break;
			}
			// accept client
			if (fd.fd == sock_listen) {
				int client_fd;
				char log_val[256];
				struct sockaddr_in client;
				client_fd = add_fds(sock_listen, &client);

				// ip port 登録
				snprintf(user[client_fd].ip_port, sizeof(user[client_fd].ip_port), "%s:%d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
				snprintf(log_val, sizeof(log_val), "'%s', current_timestamp", user[client_fd].ip_port);
				insert_log(log_val, "access_log");
			// clients process
			} else {
				// if client login, message process
				if(user[fd.fd].login_flag) {
					char log_val[1280];
					memset(msg, 0, sizeof(msg));
					// If return value == -1, error process
					if(recv_clients(i, msg) == -1) {
						break;
					}

					msg[strlen(msg)-1] = '\0';
					printf("(%s): %s\n", user[fd.fd].client_name, msg);

					// insert log
					snprintf(log_val, sizeof(log_val), "'%s', '%s', '%s', current_timestamp", user[fd.fd].client_name, msg, user[fd.fd].ip_port);
					insert_log(log_val, "msg_log");

					// process msg
					if(msg[0] == '\\') {
						process_command(i, msg);
					} else {
						broadcast_clients(i, msg);
					}
				// else login process
				} else {
					login(i);
				}
			}
		}
		
		// If client closed, must compress fds array.
		if (compress_array_flag) {
			compress_array_flag = 0;
			compress_array();
		}
	} while(!end_server);
	close_all_fds();
}

int main(int argc, char** argv) {
	int sock_listen;

	if(argc != 2) {
		printf("usage: %s [port]\n", argv[0]);
		exit(-1);
	}

	sock_listen = exp1_tcp_listen(atoi(argv[1]));
	
	init_clients(sock_listen);
	do_server(sock_listen);

	printf("end\n");

	close(sock_listen);

	return 0;
}

// recv_clients
int recv_clients(int fd_num, char msg[]) {
	char buf[1024];
	size_t len;
	struct pollfd fd = get_fd(fd_num);

	memset(buf, 0, sizeof(buf));
	len = recv(fd.fd, buf, sizeof(buf), 0);
	if(is_recv_error(len, fd_num)) {
		return -1;
	}

	strncpy(msg, buf, strlen(buf));
	
	return 0;
}

// send message to one client
void send_to(char to[], char msg[]) {
	int i;
	int ret;
	char msg_color[1024] = "\033[34m";

	for(i = 0; i < MAX_CLIENTS; i++) {
		if(strcmp(user[i].client_name, to) == 0) {
			break;
		}
	}

	if(i == MAX_CLIENTS) {
		printf("\033[33mpush msg in queue\033[39m\n");
		push_msg_queue(to, msg);
		return;
	}

	strcat(msg_color, msg);
	strcat(msg_color, "\033[39m\n");

	ret = send(i, msg_color, strlen(msg_color), 0);
	if(ret < 0) {
		perror("\033[31m! send() failed\033[39m");
	}
}

// broadcast_clients
void broadcast_clients(int fd_num, char buf[]) {
	int i;
	char msg[1024];
	size_t len;
	int ret;
	struct pollfd fd;

	snprintf(msg, sizeof(msg), "(%s): %s\n", user[get_fd(fd_num).fd].client_name, buf);
	len = strlen(msg);

	//printf("msg: %s %zu %s\n", buf, len, user[fds[fd_num].fd].client_name);
	for(i = 1; i < get_nfds(); i++) {
		fd = get_fd(i);
		if(i == fd_num) {
			continue;
		}

		if(!user[fd.fd].login_flag) {
			continue;
		}

		ret = send(fd.fd, msg, len, 0);
		if(ret < 0) {
			perror("\033[31m! send() failed\033[39m");
		}
	}
}

// recv error process
int is_recv_error(int len, int fd_num) {
	if (len < 0) {
		perror("\033[31m! recv() failed\033[39m");
		return 1;
	}

	if (len == 0) {
		shut_client(fd_num);
		return 1;
	}

	return 0;
}

// login process
void login(int fd_num) {
	const char  login_msg[] = "\033[32mLogged in!\033[39m\n";
	const char  error_msg[] = "\033[31mCould not login!\033[39m\n";
	char  username[64];
	char  password[34];
	char log_val[256];
	int   ret = 0;
	struct pollfd fd = get_fd(fd_num);

	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));

	// recv username
	ret = recv(fd.fd, username, sizeof(username), 0);
	
	if(is_recv_error(ret, fd_num)) {
		return;
	}

	username[strlen(username)-1] = '\0';

	// recv password
	ret = recv(fd.fd, password, sizeof(password), 0);

	if(is_recv_error(ret, fd_num)) {
		return;
	}

	printf("\033[35m[%s try to login]\033[39m\n", username);

	snprintf(log_val, sizeof(log_val), "'%s', '%s', current_timestamp", username, user[fd.fd].ip_port);
	insert_log(log_val, "try_to_login_log");

	// userテーブルに行が一行であれば、ログイン成功
	ret = get_row_num(username, password);
	if(ret == 1) {
		printf("\033[32m[%s logged in!]\033[39m\n", username);

		// ユーザ　初期化
		send(fd.fd, login_msg, sizeof(login_msg), 0);
		user[fd.fd].login_flag = 1;
		strncpy(user[fd.fd].client_name, username, strlen(username));
		user[fd.fd].client_name[strlen(username)+1] = '\0';

		// キュー内のDMを送る
		send_queue(fd_num, username);
	} else {

		send(fd.fd, error_msg, sizeof(error_msg), 0);
	
	}
}

// When client login, send dm message in queue
void send_queue(int fd_num, char to[]) {
	char msg[1024];

	while(1){
		memset(msg, 0, sizeof(msg));
		if(search_msg_queue(to, msg)) {
			printf("\033[34m%s\033[39m\n", msg);
			send_to(user[get_fd(fd_num).fd].client_name, msg);
		} else {
			break;
		}
	}
}