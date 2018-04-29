#include "exp1.h"
#include "exp1lib.h"
#include <pthread.h>
#include <openssl/md5.h>

char* compute_md5(char *data, char *mdString) {
    MD5_CTX c;
    unsigned char md[MD5_DIGEST_LENGTH];
    int r, i;
    
    r = MD5_Init(&c);
    if(r != 1) {
        perror("init");
        exit(1);
    }
    
    r = MD5_Update(&c, data, strlen(data));
    if(r != 1) {
        perror("update");
        exit(1);
    }
    
    r = MD5_Final(md, &c);
    if(r != 1) {
        perror("final");
        exit(1);
    }
 
    for(i = 0; i < 16; i++)
         sprintf(&mdString[i * 2], "%02x", (unsigned int)md[i]);

    return mdString;
}

void login(int sock) {
	int     ret = 0;
	char    buf[64];
    char    md5_pass[33];
    
    while(1){
        printf("username: ");
        memset(buf, 0, sizeof(buf));
        fgets(buf, 1024, stdin);
        if(strlen(buf) == 0) {
            break;
        }
        send(sock, buf, strlen(buf), 0);

        printf("password: ");
        memset(buf, 0, sizeof(buf));
        memset(md5_pass, 0, sizeof(md5_pass));
        fgets(buf, 1024, stdin);
        if(strlen(buf) == 0) {
            break;
        }
        buf[strlen(buf)-1] = '\0';
        // hash化
        compute_md5(buf, md5_pass);
        send(sock, md5_pass, sizeof(md5_pass), 0);

        memset(buf, 0, sizeof(buf));
        ret = recv(sock, buf, sizeof(buf), 0);
        if(ret <= 0) {
            perror("\033[31mrecv error\033[39m");
            break;
        }
        
        printf("%s", buf);

        if((strstr(buf, "Logged in!") - buf) > 0)
            return;
    }
}

int main(int argc, char** argv) {
	int     sock;
	int     ret;
	char    recv_buf[1024];
    char    send_buf[1024];

    pid_t   pid;

	if(argc != 3) {
		printf("usage: %s [ip address] [port]\n", argv[0]);
		exit(-1);
	}

	sock = exp1_tcp_connect(argv[1], atoi(argv[2]));
    printf("connected to %s:%s\n", argv[1], argv[2]);

    login(sock);

    /* 例えば、入力と出力をプロセス毎に分けることで、
       入力、出力で重い処理をする時、それぞれの処理を独立して行うため、そこに利点がある。

    */
    pid = fork();
    if (pid == 0) {
        // 送信(入力)担当
        while(1) {
            fgets(send_buf, 1024, stdin);
            if(strlen(send_buf) == 0) {
                break;
            }
            send(sock, send_buf, sizeof(send_buf), 0);
        }
        _exit(0);
    } else {
        // 受信担当
        while(1) {
            memset(recv_buf, 0, sizeof(recv_buf));
            ret = recv(sock, recv_buf, sizeof(recv_buf), 0);

            if(ret <= 0) {
                perror("\033[31mrecv error\033[39m");
                break;
            }
            printf("%s", recv_buf);
        }
    }

    printf("end\n");

    close(sock);

	return 0;
}