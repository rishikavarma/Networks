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
int sockfd,len;
struct sockaddr_in servaddr,cliaddr;
vector<int> rtt,packlen;
int port,seqnum,maxpacklen,maxpa,wsize,bsize,acked=0,palen,nseq=0;
double inter;
bool debug=false;
vector<pair<int,int>> buffer;
time_t startt;
int s_unack=0,w_end=0,totalpacks=0,unackpack=0,buff_end=0;
map<int,int> att;
map<int,bool> ackpa;
map<int,time_t> gentime,sendtime;
pthread_mutex_t sendl;
pthread_mutex_t recvl;
pthread_mutex_t addl;
int m=0;
bool stop=false;
struct args{	
	int seq;
	const char* buff;
	int i;
	int ptlen;
};
float average_pl(){
	float sum=0;
	for(int h=0;h<packlen.size();h++){
		sum=sum+packlen[h];
	}
	return sum/(packlen.size());
}
float average_rtt(){
	float sum=0;
	for(int h=0;h<rtt.size();h++){
		sum=sum+rtt[h];
	}
	return sum/(rtt.size());
}
void* presend(void* a){
	int seq=((struct args*)a) -> seq;
	int ptlen=((struct args*)a) -> ptlen;
	char buff[ptlen];
	const char* t1=((struct args*)a) -> buff;
	strcpy(buff,t1);
	int i=((struct args*)a) -> i;
	struct timeval t{};
	if(i<10){
		while(acked<maxpa){
			sleep(0.3);
			if(!ackpa[seq]){
				pthread_mutex_lock(&recvl);
				att[seq]++;
				totalpacks++;
				sendto(sockfd, (char *)buff,ptlen , MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
				if(att[seq]>10){
					stop=true;
					break;
				}
				pthread_mutex_unlock(&recvl);
			}
			else{
				break;
			}
		}

	}
	else{
		float rtt_av=average_rtt();
		float t1=rtt_av*10.0/1000000;
		while(acked<maxpa){
			sleep(t1);
			if(!ackpa[seq]){
				pthread_mutex_lock(&recvl);
				att[seq]++;
				totalpacks++;
				sendto(sockfd, (char *)buff,ptlen , MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
				if(att[seq]>10){
					stop=true;
					break;
				}
				pthread_mutex_unlock(&recvl);
			}
			else{
				break;
			}
		}
	}


}
void* psend(void*){
	struct timeval t{};
	while(acked<maxpa&&!stop){
		while(w_end<buff_end&&w_end-s_unack<wsize){
			int i=w_end;
			string t1=to_string(buffer[i].first);
			char buff[buffer[i].second];
			strcpy(buff,t1.c_str());
			pthread_mutex_lock(&sendl);
			sendto(sockfd, (char *)buff,buffer[i].second , MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
			gettimeofday(&t, nullptr);
			pthread_mutex_unlock(&sendl);
			time_t now = (t.tv_sec * 1000000) + (t.tv_usec);
			sendtime[buffer[i].first]=now-startt;
			packlen.push_back(buffer[i].second);
			pthread_mutex_lock(&recvl);	
			totalpacks++;		
			att[buffer[i].first]=1;
			pthread_mutex_unlock(&recvl);
			struct args* a = (struct args *)malloc(sizeof(struct args));
			a -> seq= buffer[i].first;
			a -> buff = buff;
			a -> i= i;
			a->ptlen=buffer[i].second;
			pthread_t rese;
			pthread_create(&rese, NULL, &presend,(void*)a);
			w_end++;
		}
	}
}
void* precv(void*){
	struct timeval t{};
	while(acked<maxpa&&!stop){
		char buff[MAX];
		bzero(buff,sizeof(buff));
		recvfrom(sockfd, (char *)buff, sizeof(buff), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);		
		gettimeofday(&t, nullptr);
		time_t now = (t.tv_sec * 1000000) + (t.tv_usec);
		stringstream tokens(buff);
		vector<string> words;
		string t;
		while(getline(tokens,t,' ')){
			words.push_back(t);
		} 
		int seq=stoi(words[1]);
		pthread_mutex_lock(&recvl);	
		ackpa[seq]=true;			
		pthread_mutex_unlock(&recvl);
		int rtv=now-startt-sendtime[seq];
		rtt.push_back(rtv);
    	unackpack--;
    	int pns=nseq;
		while(ackpa[nseq]){
			nseq++;
			nseq%=seqnum;
		}
		if(pns>nseq)m++;
		s_unack=nseq+m*128;
		if(debug){
			int a=int(gentime[seq]/1000);
			int b=int(gentime[seq]-(a*1000));
			cout << "Seq "<< seq <<": Time Generated: "<< a<< ":" << b<< " RTT:"<< rtv<<" Number of Attempts: "<<att[seq]<<endl;
		}
		acked++;
	}
}
int main(int argc,char* argv[]){
	
	bool cde=false,cnam=false,cpor=false,cseq=false,cmpl=false,cpgr=false,cmp=false,cws=false,cbs=false;
	char* name;
	int seq=0,rate;
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
		else if(!temp.compare("-s")&&!cnam){
			i++;
			name=argv[i];
			i++; 
			cnam=true;
		}
		else if(!temp.compare("-L")&&!cmpl){
			i++;
			maxpacklen=stoi(argv[i]);
			i++; 
			cmpl=true;
		}
		else if(!temp.compare("-R")&&!cpgr){
			i++;
			rate=stoi(argv[i]);
			inter=1.0/rate;
			inter=inter*1000000.0;
			i++; 
			cpgr=true;
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
	if(!cnam||!cpor||!cseq||!cmpl||!cpgr||!cmp||!cws||!cbs){
		cout<<"Input Error.\n";
		exit(0);
	}
	
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        cout<<"Socket creation failed."<<endl;
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8080);
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        cout<<"Bind failed."<<endl;
        exit(0);
    }
    cliaddr.sin_family    = AF_INET; 
    inet_pton(AF_INET,name, &cliaddr.sin_addr);
	cliaddr.sin_port = htons(port);
	len = sizeof(cliaddr);
	struct timeval t{};
	gettimeofday(&t, nullptr);
	startt = (t.tv_sec * 1000000) + (t.tv_usec);
	pthread_t p1,p2;
    pthread_create(&p1, NULL, &psend,NULL);
    pthread_create(&p2, NULL, &precv, NULL);
    sleep(3);
    default_random_engine generator;
	uniform_int_distribution<int> distribution(40,maxpacklen);
    while(acked<maxpa&&!stop){
    	if(unackpack<bsize){
    		int psi=distribution(generator);
    		buffer.push_back(make_pair(seq,psi));
    		gettimeofday(&t, nullptr);
			time_t temt= (t.tv_sec * 1000*1000) + (t.tv_usec);
			gentime[seq]=(temt-startt);
			ackpa[seq]=false;
    		buff_end++;
			pthread_mutex_lock(&addl);
    		unackpack++;
			pthread_mutex_unlock(&addl);
    		seq++;
    		seq%=seqnum;
    	}
    	usleep(inter);
    }
    close(sockfd);
    sleep(2);
    cout << "PACKET_GEN_RATE : " << rate << endl;
	cout << "PACKET_LENGTH : " << average_pl() << endl;
	cout << "Retransmission ratio : " << totalpacks*1.0/acked << endl;
	cout << "RTT average : " << average_rtt()<< endl;
    return 0;

}