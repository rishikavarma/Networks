#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <ctime>
using namespace std;
#define MAX 10000

int main(int argc,char* argv[]){
	bool debug=false;
	bool cde=false,cpor=false,cer=false,cseq=false,cmp=false,cws=false,cbs=false;
	string name;
	int port,seqnum,maxpa,wsize,bsize;
	double erate;
	for(int i=1;i<argc;){
		string temp(argv[i]);
		if(!temp.compare("-d")&&!cde){
			i++;
			debug=true;
			cde=true;
		}		
		else if(!temp.compare("-p")&&!cpor){
			i++;
			port=stoi(argv[i]);
			i++; 
			cpor=true;
		}
		else if(!temp.compare("-n")&&!cseq){
			i++;
			int t1=stoi(argv[i]);
			seqnum=pow(2,t1-1);
			i++; 
			cseq=true;
		}
		else if(!temp.compare("-e")&&!cer){
			i++;
			erate=stod(argv[i]);
			i++; 
			cer=true;
		}
		
		else if(!temp.compare("-N")&&!cmp){
			i++;
			maxpa=stoi(argv[i]);
			i++; 
			cmp=true;
		}
		else if(!temp.compare("-W")&&!cws){
			i++;
			wsize=stoi(argv[i]);
			i++; 
			cws=true;
		}
		else if(!temp.compare("-B")&&!cbs){
			i++;
			bsize=stoi(argv[i]);
			i++; 
			cbs=true;
		}
		else{
			cout<<"Input Error.\n";
			exit(0);
		}
	}
	if(!cpor||!cer||!cseq||!cmp||!cws||!cbs){
		cout<<"Input Error.\n";
		exit(0);
	}
	default_random_engine generator;
	bernoulli_distribution distribution(erate);
	int sockfd,len;
	struct sockaddr_in servaddr,cliaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	int totrecp=0;	
	int w_start=0,w_end=wsize-1,actpacks=0;
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        cout<<"Socket creation failed."<<endl;
        exit(0);
    }
	servaddr.sin_family    = AF_INET; 
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);
	len = sizeof(cliaddr);
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        cout<<"Bind failed."<<endl;
        exit(0);
    } 
    struct timeval t{};
	gettimeofday(&t, nullptr);
	time_t startt;
	vector<bool> recvd;
	recvd.resize(maxpa,false);
	vector<int> buffer;
	for(int i=0;i<maxpa;i++){
		buffer.push_back(i%seqnum);
	}
	startt = (t.tv_sec * 1000000) + (t.tv_usec);
	while(totrecp<maxpa){
		char buff[MAX];
		bzero(buff,sizeof(buff));		
		recvfrom(sockfd, (char *)buff,sizeof(buff) , MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
		gettimeofday(&t, nullptr);
		time_t now = (t.tv_sec * 1000000) + (t.tv_usec);
		if(!distribution(generator)){
			stringstream tokens(buff);
			vector<string> words;
			string t1;
			while(getline(tokens,t1,' ')){
				words.push_back(t1);
			} 
			int seq=stoi(words[0]);
			if(actpacks>=bsize && seq!=buffer[w_start])continue;
			int don=0;
			for(int j=w_start;j<w_end;j++){
				if(buffer[j]==seq){
					if(recvd[j]){
						break;
					}
					recvd[j]=true;
					don=1;
					break;
				}
			}
			if(don==0)continue;			
			char buff1[MAX]="ACK ";
			strcat(buff1,buff);
			sendto(sockfd, (char *)buff1, strlen(buff1), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
			actpacks++;
			totrecp++;
			int a=int((now-startt)/1000);
			int b=int((now-startt) - (a*1000));
			if(debug)cout << "Seq "<<seq<<": Time Received:"<<a<<":"<<b<<endl;
			while(buffer.size()>0&&recvd[w_start]){
				buffer.erase(buffer.begin()+w_start);
				recvd.erase(recvd.begin()+w_start);
				actpacks--;
				if(w_end>buffer.size())w_end=buffer.size();
			}
		}

	}
	close(sockfd);
	return 0;
	
}