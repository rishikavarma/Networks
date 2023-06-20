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
	bool cde=false,cpor=false,cer=false,cmp=false,cseq=false;
	string name;
	int port,seqnum,maxpa;
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
		else if(!temp.compare("-n")&&!cseq){
			i++;
			int t1=stoi(argv[i]);
			seqnum=pow(2,t1-1);
			i++; 
			cseq=true;
		}
		else{
			cout<<"Input Error.\n";
			exit(0);
		}
	}
	if(!cpor||!cer||!cmp||!cseq){
		cout<<"Input Error.\n";
		exit(0);
	}
	default_random_engine generator;
	bernoulli_distribution distribution(erate);
	int sockfd,len;
	struct sockaddr_in servaddr,cliaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
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
	startt = (t.tv_sec * 1000000) + (t.tv_usec);
	int totrecp=0,actpacks=0;
	int nextseq=0;
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
			if(seq!=nextseq) continue;
			char buff1[MAX]="ACK ";
			strcat(buff1,buff);
			sendto(sockfd, (char *)buff1, strlen(buff1), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
			totrecp++;
			int a=int((now-startt)/1000);
			int b=int((now-startt) - (a*1000));
			nextseq=(nextseq+1)%seqnum;
			
			if(debug)cout << "Seq "<<seq<<": Time Received:"<<a<<":"<<b<<endl;
		}
	}	

	close(sockfd);
	return 0;
}