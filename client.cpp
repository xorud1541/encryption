#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void * send_msg(void* arg);
void * recv_msg(void* arg);
void error_handling(char *arg);

char nick[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char* argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc != 3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

	while(1){

		/* interface */
		int num = 0;
		int flag = -1;
		printf("1. [join] \n");
		printf("2. [log in] \n");
		printf("3. [quit] \n");
		scanf("%d", &num);
		getchar();
		if(num == 1){
			char ack[4];
			char msg[120];
			char name[20];
			char pwd[20];
			char temp[20];
			fflush(stdin);
			fputs("ID >> ", stdout);
			scanf("%s", name);
			
			fputs("PWD >> ", stdout);
			scanf("%s", pwd);
			
			sprintf(msg, "1 %s %s", name, pwd);
			strcpy(temp, name);
		        write(sock, msg, strlen(msg));

			int len = 0;
			while(1) {
				len = read(sock, ack, 4);
				if(len != -1){
				     ack[len] = 0;
				     break;
				}
			}

			if(!strcmp(ack, "yes")){
				flag = 1;
				printf("\n\njoin success!!\n");
				printf("you can use a service right now!\n");
				strcpy(nick, temp);
			}

			else{
				printf("join fail!!\n");
			}

			if(flag > 0) break;
			else
				continue;


		}
		else if(num == 2){
			/* log in
			 * if log in success, change flag = 1;
			 *
			 *
			 * end */


		}
		else {

			/* num == 3
			 * quit
			 *
			 * close(sock);
			 * return 0; */

		}
	}

	printf("***************CHAT SERVICE******************\n");

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	return 0;
}

void * send_msg(void * arg){
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE+2];

	while(1){
		fgets(msg, BUF_SIZE, stdin);
		if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")){
			close(sock);
			exit(0);
		}
		sprintf(name_msg, "[%s]: %s", nick, msg);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}

void * recv_msg(void * arg){
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1){
		str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len == -1)
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
	}
	return NULL;
}

void error_handling(char * msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

