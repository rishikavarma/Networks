#include <iostream>
#include <fstream> 
#include <bits/stdc++.h>
#include <string.h>    
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   
#include <arpa/inet.h>    
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <pthread.h>
#include <sys/time.h>
using namespace std;
#define MAXLINE 1024
#define PORT 8080

struct arguments{
	const char* buf;
	int seq_no;
	int pkt_no;
};
string recvr_name;
bool debug = false;
int seq_no = 0,pkt_no = 0;
int port,PACKET_LENGTH,MAX_PACKETS;
float PACKET_GEN_RATE;
int WINDOW_SIZE,MAX_BUFFER_SIZE,seq_no_len;
struct sockaddr_in servaddr,cliaddr;
int sockfd,len,ack_rcvd = 0;
pthread_mutex_t lock_for_send;
pthread_mutex_t lock_for_ack;
vector<int>unack_buf;
vector<time_t>sending_time;
vector<bool>mark;
vector<int>transmission;
vector<int>attempts;
time_t start_usec;
vector<float>rtt;
float RTT = 100000;
int unack_buf_size = 0;

float get_average(){
	int size = 0,sum = 0;
	for (int i = 0;i<rtt.size();i++){
		if(rtt[i]!=0){
			size++;
			sum+=rtt[i];
		}
	}
	return (sum/size);
}

void* resend(void* args){
	int seq_no = ((struct arguments*)args) -> seq_no;
	const char* buf = ((struct arguments*)args) -> buf;
	int pkt_no = ((struct arguments*)args) -> pkt_no;
	int timer_time = 0;
	if(pkt_no < 10) timer_time = 100000;
	else timer_time = 2*RTT;
	while(1){
		usleep(timer_time);
		if(!mark[seq_no]){
			int jj = pkt_no;
			for(int i = 0; i < WINDOW_SIZE; i++,jj++){
				int seq_num = unack_buf[jj];
				string buffer = to_string(seq_num);
				//cout << "Sending Packet " << seq_num << endl;
				const char* buf = (const char*)malloc(PACKET_LENGTH);
				buf = buffer.c_str();
				pthread_mutex_lock(&lock_for_send);
				sendto(sockfd, (char *)buf, strlen(buf), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
				pthread_mutex_unlock(&lock_for_send);
				pthread_mutex_lock(&lock_for_ack);
				transmission[jj]++;
				attempts[seq_num]++;
				pthread_mutex_unlock(&lock_for_ack);
			}		
		}
		else{
			break;
		}
	}
	return 0;
}

void* gen_packet(void*){
	sleep(5);
	int seq_no = 0;
	while(1){
		if(unack_buf_size<MAX_BUFFER_SIZE){
			unack_buf.push_back(seq_no);
			unack_buf_size++;
			struct timeval time_now{};
			gettimeofday(&time_now, nullptr);
			time_t msecs_time = (time_now.tv_sec * 1000*1000) + (time_now.tv_usec);
			sending_time[seq_no] = msecs_time - start_usec; // time elapsed in microsecs from the start of the pgrm
			seq_no = (seq_no+1)%(seq_no_len);
			pkt_no++;
		}
		usleep(PACKET_GEN_RATE*1000000);
	}
}

void* send_packet(void*){
	int jj = 0;
	int i = 0;
	while(1){
		for(; i < unack_buf.size() && unack_buf[i] < jj + WINDOW_SIZE; i++){
			int seq_num = unack_buf[i];
			string buffer = to_string(seq_num);
			const char* buf = (const char*)malloc(PACKET_LENGTH);
			buf = buffer.c_str();
			// cout << "Packet "<<buf<<endl;
			pthread_mutex_lock(&lock_for_send);
			sendto(sockfd, (char *)buf, strlen(buf), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
			pthread_mutex_unlock(&lock_for_send);
			pthread_mutex_lock(&lock_for_ack);
			transmission[i]++;		// number of transmissions
			attempts[seq_num]++;
			pthread_mutex_unlock(&lock_for_ack);
			struct arguments* args = (struct arguments *)malloc(sizeof(struct arguments));
			args -> seq_no = seq_num;
			args -> buf = buf;
			args -> pkt_no = i;
			pthread_t timer;
			pthread_create(&timer, NULL, &resend,(void*)args);// thread that runs timer
			if(mark[jj]){
				jj++;
			}
			if(i > 1 && i%seq_no_len == 0){
				mark.clear();
				mark.resize(seq_no_len);	
			}
		}
	}
	return 0;
}

void* ack_recv(void*){
	while(1){
		char buffer[MAXLINE];
		memset(&buffer, 0, MAXLINE);
		recvfrom(sockfd, (char *)buffer, PACKET_LENGTH, MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
		struct timeval time_now{};
		gettimeofday(&time_now, nullptr);
		time_t msecs_time = (time_now.tv_sec * 1000*1000) + (time_now.tv_usec);
		vector<string> tokens;
		stringstream tokenize(buffer);
		string temp;
		while(getline(tokenize,temp,' ')) tokens.push_back(temp);
		int ack_no = stoi(tokens[1]);
		mark[ack_no] = true;
		rtt[ack_no] = msecs_time - start_usec - sending_time[ack_no];
		RTT = get_average();
		unack_buf_size--;
		if(debug){
			int xx = int(sending_time[ack_no]/1000);
			int yy = int(sending_time[ack_no] - (xx*1000));
			pthread_mutex_lock(&lock_for_ack);
			cout << "Seq "<< ack_no <<": Time Generated: "<<xx << ":" <<yy<< " RTT:"<< rtt[ack_no]<<" Number of Attempts: "<<attempts[ack_no]<<endl;
			
			if(ack_no == 0){
				attempts.clear();
				attempts.resize(seq_no_len);
			}
			pthread_mutex_unlock(&lock_for_ack);
		}
		ack_rcvd++;
	}
	return 0;
}

int main(int argc,char* argv[]){
	int all = 0;
	for(int j = 1;j!=argc-1;){
		switch(argv[j++][1]){
			case('d'): debug = true;break;
			case('s'): recvr_name = argv[j];all++;break;
			case('p'): port = stoi(argv[j]);all++;break;
			case('l'): PACKET_LENGTH = stoi(argv[j]);all++;break;
			case('r'): PACKET_GEN_RATE = stoi(argv[j]);all++;break;
			case('n'): seq_no_len = stoi(argv[j]);all++;break;
			case('N'): MAX_PACKETS = stoi(argv[j]);all++;break;
			case('w'): WINDOW_SIZE = stoi(argv[j]);all++;break;
			case('b'): MAX_BUFFER_SIZE = stoi(argv[j]);all++;break; 
		}
	}
	if(all!=8){
		cout << "Input Error\n";
		exit(0);
	}
	seq_no_len = pow(2,seq_no_len-1);
	PACKET_GEN_RATE = 1/PACKET_GEN_RATE;
	sending_time.resize(seq_no_len);
	mark.resize(seq_no_len,false);
	transmission.resize(MAX_PACKETS);
	attempts.resize(seq_no_len);
	rtt.resize(MAX_PACKETS);
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	    perror("socket creation failed");
	    exit(EXIT_FAILURE);
	}
	const char* recvr = recvr_name.c_str();
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	// Filling server information
	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(8080);
	cliaddr.sin_family    = AF_INET; // IPv4
	inet_pton(AF_INET,recvr, &cliaddr.sin_addr);
	cliaddr.sin_port = htons(port);
	len = sizeof(cliaddr);
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
	    perror("bind failed");
	    exit(EXIT_FAILURE);
	}
	struct timeval time_now{};
	gettimeofday(&time_now, nullptr);
	start_usec = (time_now.tv_sec * 1000*1000) + (time_now.tv_usec);
	pthread_t pkt_gen;
	pthread_create(&pkt_gen, NULL, &gen_packet,NULL);
	pthread_t pkt_send;
	pthread_create(&pkt_send, NULL, &send_packet,NULL);
	pthread_t recv;
	pthread_create(&recv, NULL, &ack_recv,NULL);
	while(ack_rcvd < MAX_PACKETS);
	int sum_transmission = 0;
	for(int i = 0; i< transmission.size();i++){
		sum_transmission += transmission[i];
		// cout << transmission[i] << " ";
	}
	float Retransmission_ratio = sum_transmission*1.0/ack_rcvd;
	cout << "PACKET_GEN_RATE = " << PACKET_GEN_RATE << endl;
	cout << "MAX_PACKET_LENGTH = " << PACKET_LENGTH << endl;
	cout << "Retransmission ratio = " << Retransmission_ratio << endl;
	int xx = RTT/1000;
	int yy = RTT - xx;
	cout << "RTT average = " << xx <<":" <<yy<< endl;
	return 0;
}