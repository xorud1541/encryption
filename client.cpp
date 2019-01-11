#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#define BUF_SIZE 100
#define NAME_SIZE 20
using namespace std;
void * send_msg(void* arg);
void * recv_msg(void* arg);
void error_handling(char *arg);

//char nick[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
string nick;
//string msg;
int interface(int);
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
		int s = interface(sock);
		if(s==1){
			cout << "success" << endl;
			break;
		}
		else if(s==-1)
			continue;
		else
		{
			return 0;
		}
	}

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	return 0;
}

int interface(int sock){
	int num;
	printf("1. [join]\n");
	printf("2. [login]\n");
	printf("3. [quit]\n");
	cin >> num;


	if(num == 1){
		string id;
		string pwd;
		printf(" ID >> ");
		cin >> id;
		printf(" PWD >> ");
		cin >> pwd;

		string str = "1";
		str = str + ' ' + id + ' ' + pwd;

		/* send id and password i would like */

		write(sock, str.c_str(), str.size());
		
		/* check message from server */
		
		char ack[4];
		string temp;
		int len = read(sock, ack ,4);
		if(len != -1){
			 ack[len] = 0;
			 temp = string(ack);
		}


		if(temp.compare("yes") == 0){
			nick = id;
			return 1;
		}
		else
			return -1;
					
	}
	else if(num == 2){
	}
	else if(num==3){
		return 0;
	}
}

void * send_msg(void * arg){
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE+2];
	while(1){
		fgets(msg, sizeof(msg), stdin);
	
		sprintf(name_msg, "[%s]: %s", nick.c_str(), msg);
		write(sock, name_msg, sizeof(name_msg)-1);
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

