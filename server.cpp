#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <vector>
#include <list>
#include <unistd.h>
#include <iostream>
#include <map>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#define BUF_SIZE 100
#define MAX_CLNT 256
using namespace std;
void * handle_clnt(void * arg);
void send_msg(char* msg, int len, int);
void error_handling(char* msg);


vector< pair<int, int> > clnt_socks; /* save all client sock information */
map<string, string> before_clnt_list;
map<string, string> after_clnt_list;
ofstream output;
pthread_mutex_t mutx;

int main(int argc, char* argv[]){
	ifstream input;
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	pthread_t t_id;
	
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	/*****************************************
	 *
	 * load client textfile
	 *
	 * ID Password
	 *
	 * use a map data-structre
	 *
	 * -------------------
	 * |  ID    |  PWD   |     
	 * -------------------
	 *
	 * ***************************************/
	
	input.open("list.txt");
	
	while(!input.eof()){
		string id;
		string pwd;
		input >> id;
		input >> pwd;

		before_clnt_list.insert(make_pair(id, pwd));
	}

	input.close();
		
	output.open("list.txt", ios::app);

	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while(1){
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		pthread_mutex_lock(&mutx);
		clnt_socks.push_back(make_pair(clnt_sock, -1)); /*client socket, -1 -> not permitted service  */
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connectd client IP : %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);

	output.close();

	return 0;
}


void * handle_clnt(void * arg){
	int clnt_sock = *((int*)arg);
	int str_len = 0;
	char msg[BUF_SIZE];
	int room = 0;
	//int len = read(clnt_sock, msg, sizeof(msg));
	//if(len != -1){
	bool ok = false;
	while(1){
		int len = read(clnt_sock, msg, sizeof(msg));
		if(len != -1){

			cout << "test 11 " << endl;
			string s = string(msg);
		
			vector<string> info; //client information
			string temp;
			for(int i=0; i<s.size(); i++){
				if(s[i] == ' '){
					info.push_back(temp);
					temp.clear();
				}
				else{
					temp += s[i];
				}
			}

			/*************************
		 	* client info
		 	* DataType
		 	* #1 join / login / quit
		 	* #2 ID
		 	* #3 PWD
		 	* #4 room
		 	**************************/

			info.push_back(temp);
			temp.clear();
		
			if(info[0] == "1"){
				// join	
				string id = info[1];
				string pwd = info[2];
			
				/**********************
				 *
			 	* check ID and PWD here
			 	*
			 	* ********************/

				int before = before_clnt_list.count(id);
				int after = after_clnt_list.count(id);
				if(before == 0 && after == 0){

					pthread_mutex_lock(&mutx);
					after_clnt_list.insert(make_pair(id, pwd));
					output << id << ' ' << pwd << endl;
			
					// success joining on chat service
					for(int i=0; i<clnt_socks.size(); i++){
						if(clnt_socks[i].first == clnt_sock){
							clnt_socks[i].second = 0; /* move on room 0 */
							break;
						}
					}
					pthread_mutex_unlock(&mutx);

					write(clnt_sock, "yes", sizeof("yes")); // ack //
					ok = true;
				}
				else
				{
					write(clnt_sock, "no", sizeof("no"));

				}

	
			}
			else if(info[0] == "2"){
				// login
			}
			else if(info[0] == "3"){
				// quit
			}

		} //len

		if(ok) break;		
	}


	char chatrm[2];
	int temp = read(clnt_sock, chatrm, sizeof(chatrm));
	string chatroom = string(chatrm);

	pthread_mutex_lock(&mutx);
	if(chatroom.compare("1") == 0){
		for(int i=0; i<clnt_socks.size(); i++){
			if(clnt_socks[i].first == clnt_sock){
				clnt_socks[i].second = 1;
				room = 1;
				break;
			}
		}
	}
	else{
		for(int i=0; i<clnt_socks.size(); i++){
			if(clnt_socks[i].first == clnt_sock){
				clnt_socks[i].second = 2;
				room = 2;
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutx);

	while((str_len = read(clnt_sock, msg, sizeof(msg))) != 0){
		send_msg(msg, str_len, room);
	}

	pthread_mutex_lock(&mutx);
	
	vector<pair<int, int>>::iterator iter;
	for(iter = clnt_socks.begin(); iter!= clnt_socks.end(); iter++){
		if((iter->first) == clnt_sock){
			clnt_socks.erase(iter);
			break;
		}
	}
	
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void send_msg(char * msg, int len, int room){
	pthread_mutex_lock(&mutx);
	int size = clnt_socks.size();
	for(int i=0; i<size; i++){
		int temp = clnt_socks[i].second;
		if(temp > 0 && (temp == room) && clnt_socks[i].first > 0){
			write(clnt_socks[i].first, msg, len);
		}
	}
	pthread_mutex_unlock(&mutx);
}

void error_handling(char *msg){
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

