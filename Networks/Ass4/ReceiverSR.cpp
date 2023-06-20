#include <iostream>
#include <fstream> 
// #include <bits/stdc++.h>
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
#include <vector>
#include <cmath>
using namespace std;
#define MAXLINE 1024

int main(int argc, char* argv[]){
	bool debug = false;
	int all = 0;
	int port,MAX_PACKETS,seq_no_len;
	int WINDOW_SIZE,BUFFER_SIZE;
	double PACKET_ERROR_RATE;
	int r_seq_no = 0;
	int pkt_rcvd = 0;
	for(int j = 1;j!=argc-1;){
		switch(argv[j++][1]){
			case('d'):debug = true;break;
			case('p'):port = stoi(argv[j]);all++;break;
			case('n'):seq_no_len = stoi(argv[j]);all++;break;
			case('N'):MAX_PACKETS = stoi(argv[j]);all++;break;
			case('W'):WINDOW_SIZE = stoi(argv[j]);all++;break;
			case('B'):BUFFER_SIZE = stoi(argv[j]);all++;break;
			case('e'):PACKET_ERROR_RATE = stod(argv[j]);all++;break;
		}
	}
	if(all != 6){
		cout << "Input error\n";
		exit(0);
	}
	seq_no_len = pow(2,seq_no_len-1);
	vector<bool>mark(seq_no_len);
	vector<int>rcv_buf;
	int size_of_rcv_buf = 0;
	vector<time_t>elapse(seq_no_len);
	struct sockaddr_in servaddr,cliaddr;
	int sockfd;
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	    perror("socket creation failed");
	    exit(EXIT_FAILURE);
	}
	  
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	// Filling server information
	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);
	int len = sizeof(cliaddr);
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
	    perror("bind failed");
	    exit(EXIT_FAILURE);
	}
	
	
	struct timeval start{};
	gettimeofday(&start, nullptr);
	time_t start_usec = start.tv_sec * 1000*1000 + start.tv_usec;
	while(pkt_rcvd < MAX_PACKETS){
		char buffer[MAXLINE];
		memset(&buffer, 0, MAXLINE);
		recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
		struct timeval time_now{};
		gettimeofday(&time_now, nullptr);
		time_t time_now_usec = time_now.tv_sec * 1000*1000 + time_now.tv_usec;
		int seq_no = stoi(buffer);
		if(size_of_rcv_buf >= BUFFER_SIZE){
			if(seq_no!=r_seq_no) continue;
		}
		double prob = (double)rand()/ RAND_MAX;
		if(prob < PACKET_ERROR_RATE) continue;
		else{
			if(mark[seq_no]) continue;
			string reply = "ACK " + to_string(seq_no);
			mark[seq_no] = true;
			elapse[seq_no] = time_now_usec - start_usec;
			if(r_seq_no == seq_no){
				while(mark[r_seq_no] == true){
					if(debug){
						int xx = int(elapse[r_seq_no]/1000);
						int yy = int(elapse[r_seq_no] - (xx*1000));

						cout << "Seq "<<r_seq_no<<": Time Received:"<<xx<<":"<<yy<<endl;
					}
					r_seq_no = (r_seq_no+1)%(seq_no_len);
					if(r_seq_no == 0){
						mark.clear();
						mark.resize(seq_no_len);
					}
				} 
				for(auto it = rcv_buf.begin();it!=rcv_buf.end();){
					if(*(it) < r_seq_no){
						rcv_buf.erase(it);
						size_of_rcv_buf--;
					} 
					else it++;
				}
			}
			else{
				rcv_buf.push_back(seq_no);
				size_of_rcv_buf++;
			}
			pkt_rcvd++;
			const char* buf = reply.c_str();
			sendto(sockfd, (char *)buf, strlen(buf), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
			
		}
	}
	close(sockfd);
	ofstream outfile;
	outfile.open("Error_SR.txt",ios::app);
	outfile << PACKET_ERROR_RATE << endl;
	outfile.close();
	return 0;
}