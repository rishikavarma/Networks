#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;
#define MAX 10000
struct interval{
    int time;
};
ofstream ofile;
int id;
int sockfd;
pthread_mutex_t l1,l2;
struct sockaddr_in servaddr;
vector<int> neigh;
vector<vector<pair<int,int>>> vm;
vector<vector<int>> v;
vector<int> done;
int n;
int seq=1;
const char* itoa(int k){
	return (to_string(k)).c_str();
}
void* hif(void* a){
	int inter=((interval*)a) ->time;
	while(1){
		sleep(inter);
		char buff[100]="HELLO ";
		strcat(buff,itoa(id));
		struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        int len = sizeof(cliaddr);
        for(int i=0;i<neigh.size();i++){
        	int port=10000+neigh[i];
        	cliaddr.sin_family    = AF_INET; 
            cliaddr.sin_addr.s_addr = INADDR_ANY;
            cliaddr.sin_port = htons(port);
            pthread_mutex_lock(&l1);
            sendto(sockfd, (char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
            pthread_mutex_unlock(&l1);
        }
	}
}

void* hirf(void* a){
	while(1){	
		struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        int len = sizeof(cliaddr);
        char buff[MAX],buff1[MAX];
        bzero(buff,sizeof(buff));
        recvfrom(sockfd, (char *)buff, sizeof(buff), MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
        int i=0;
		stringstream tokens(buff);
		vector<string> words;
		string t;
		while(getline(tokens,t,' ')){
			words.push_back(t);
		} 
		if(!words[0].compare("HELLO")){
			int j = stoi(words[1]);
			char temp[100]="HELLOREPLY ";
			strcat(temp,itoa(id));
			strcat(temp," ");
			strcat(temp,words[1].c_str());
			strcat(temp," ");
			int val=vm[j][id].first+(rand()%(vm[j][id].second-vm[j][id].first+1));
			strcat(temp,itoa(val));
			pthread_mutex_lock(&l1);
            sendto(sockfd, (const char *)temp, strlen(temp), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
            pthread_mutex_unlock(&l1);
        }
        else if(!words[0].compare("HELLOREPLY")){
        	int i=stoi(words[2]),j=stoi(words[1]);
        	pthread_mutex_lock(&l2);
        	v[i][j]=stoi(words[3]);
        	pthread_mutex_unlock(&l2);
        }
        else if(!words[0].compare("LSA")){
        	int ent=stoi(words[3]),seqnum=stoi(words[2]),j=stoi(words[1]);
        	if(seqnum>done[j]){
        		for(int i=4;i<(ent*2+4);i+=2){
        			int k=stoi(words[i]);
        			pthread_mutex_lock(&l2);
        			v[j][k]=stoi(words[i+1]);
        			pthread_mutex_unlock(&l2);
        		} 
        		 		
        		for(int i=0;i<neigh.size();i++){
        			if(neigh[i]!=j){
        				int port=10000+neigh[i];
		        		cliaddr.sin_family    = AF_INET; 
	                    cliaddr.sin_addr.s_addr = INADDR_ANY;
	                    cliaddr.sin_port = htons(port);
	                    pthread_mutex_lock(&l1);
	                    sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
	                    pthread_mutex_unlock(&l1);
        			}        			
        		}
        		done[j]=seqnum;
	        	seq++;
        	}
        }
        
	}
}
void* lsaf(void* a){
	int inter=((interval*)a)->time;
	struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    int len = sizeof(cliaddr);
    while(1){
    	sleep(inter);
    	pthread_mutex_lock(&l2);
        char temp[MAX]="LSA ";
	    strcat(temp,itoa(id));
	    strcat(temp," ");
	    strcat(temp,itoa(seq));
	    strcat(temp," ");
	    char temp1[MAX]="";
	    int entr=0;
	    for(int i=0;i<v[id].size();i++){
	        if(v[id][i]!=-1){
	        	strcat(temp1,itoa(i));
	        	strcat(temp1," ");
	        	strcat(temp1,itoa(v[id][i]));
	        	strcat(temp1," ");
	        	entr++;
	        }
	    }
	    strcat(temp,itoa(entr));
	    strcat(temp," ");
	    strcat(temp,temp1);     		
	    pthread_mutex_unlock(&l2);
	    for(int i=0;i<neigh.size();i++){
        	int port=10000+neigh[i];
	        cliaddr.sin_family    = AF_INET; 
            cliaddr.sin_addr.s_addr = INADDR_ANY;
            cliaddr.sin_port = htons(port);
            pthread_mutex_lock(&l1);
            sendto(sockfd, (const char *)temp, strlen(temp), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
            pthread_mutex_unlock(&l1);
        }
	    seq++;
    }
}
void djikstra(vector< pair<int,string>>& ans){
	vector<bool> reach;
	reach.resize(n,false);
	ans[id].first=0;
	ans[id].second=to_string(id);
	for(int i=0;i<n;i++){
		int min=INT_MAX,cno;
		for(int j=0;j<n;j++){
			if(!reach[j]&&min>=ans[j].first){
				min=ans[j].first;
				cno=j;
			}
		}
		reach[cno]=true;
		for(int j=0;j<n;j++){
			if (!reach[j] && v[cno][j] != -1 && ans[cno].first != INT_MAX && ans[cno].first + v[cno][j] < ans[j].first){
                ans[j].first = ans[cno].first + v[cno][j];
                ans[j].second= ans[cno].second+"-"+to_string(j);
            }
		}
	}
}
void* spff(void* a){
	int inter=((interval*)a)->time;
	struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    int len = sizeof(cliaddr);
    int tim=0;
    while(1){
    	sleep(inter);
    	tim=tim+inter;
    	vector< pair<int,string>> ans;
    	ans.resize(n,make_pair(INT_MAX,""));
        djikstra(ans);
    	ofile<<"Routing Table for Node no."<<id<<" at time "<<tim<<"s :"<<endl;
    	ofile<<endl;
        ofile<<"Destination        \tPath          \tCost"<<endl;
        for (int i = 0; i < n; i++){
            if(id==i) continue;
            if(ans[i].first!=INT_MAX)ofile<<i<<" \t\t            "<<ans[i].second<<"\t\t         "<<ans[i].first<<endl;
            else ofile<<i<<"      \t\t \tNo Path"<<endl;
        }
        ofile<<endl;
    }
}
int main(int argc, char* argv[]){
	if(argc<7){
		cout<<"Input error"<<endl;
		exit(0);
	}
	double hi=1,lsa=5,spf=20;
	ifstream ifile;
	bool cid=false,chi=false,clsa=false,cspf=false,cif=false,cof=false;
	char* of;
	for(int i=1;i<argc;){
		string temp(argv[i]);
		if(!temp.compare("-i")&&!cid){
			i++;
			id=stoi(argv[i]);
			i++; 
			cid=true;
		}		
		else if(!temp.compare("-o")&&!cof){
			i++;
			of=argv[i];
			i++; 
			cof=true;
		}
		else if(!temp.compare("-f")&&!cif){
			i++;
			string t1(argv[i]);
			ifile.open(t1+".txt");
			i++; 
			cif=true;
		}
		else if(!temp.compare("-s")&&!cspf){
			i++;
			spf=stod(argv[i]);
			i++; 
			cspf=true;
		}
		else if(!temp.compare("-h")&&!chi){
			i++;
			hi=stod(argv[i]);
			i++; 
			chi=true;
		}
		else if(!temp.compare("-a")&&!clsa){
			i++;
			lsa=stod(argv[i]);
			i++; 
			clsa=true;
		}
		else{
			cout<<"Input Error.\n";
			exit(0);
		}
	}
	if(!cof||!cif||!cid){
		cout<<"Input Error.\n";
		exit(0);
	}
	strcat(of,"-");
	const char* tid=(to_string(id)).c_str();
	strcat(of,tid);
	strcat(of,".txt");
	ofile.open(of);
	int links;
	ifile>>n>>links;
	vm.resize(n);
	v.resize(n);
	done.resize(n);
	for(int i=0;i<n;i++){
		vm[i].resize(n,make_pair(-1,-1));
		v[i].resize(n,-1);
	}
	for(int k=0;k<links;k++){
		int i,j;
		double max,min;
		ifile>>i>>j>>min>>max;
		vm[i][j].first=min;
		vm[i][j].second=max;
		vm[j][i]=vm[i][j];
		if(i==id){ 
			neigh.push_back(j);
		}
		if(j==id) {
			neigh.push_back(i);
		}

	}
	ifile.close();
	int port=10000+id;
	char buffer[MAX];
	struct sockaddr_in cliaddr;
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        cout<<"Socket creation failed."<<endl;
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        cout<<"Bind failed."<<endl;
        exit(0);
    }
    int len;  
    len = sizeof(cliaddr);
    interval* a1=(interval*)malloc(sizeof(interval));
    a1->time=hi;
    pthread_t p1,p2,p3,p4;
    pthread_create(&p1, NULL, &hif,(void*)a1);
    pthread_create(&p2, NULL, &hirf, (void*)a1);
    a1->time=lsa;
    pthread_create(&p3, NULL, &lsaf,(void*)a1);
    a1->time=spf;
    pthread_create(&p4, NULL, &spff, (void*)a1);
    while(1);
	ofile.close();
}