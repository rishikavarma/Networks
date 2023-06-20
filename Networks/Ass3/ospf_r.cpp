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
using namespace std;
#define TRUE   1 
#define FALSE  0 
#define MAXLINE 1024
ifstream file;
ofstream outfile;
vector<vector<int>> v;
vector<vector<int>> graph;
vector<int> neighbors;
int N,M;
int graphsize = 0;
int seq_no = 1;
pthread_mutex_t lock1,lock2;
vector<int> known_seq_no;
int number = 0;
struct arguments{
    int id;
    int sockfd;
    struct sockaddr_in servaddr;
    int interval;
    char* filename;
};

void* hello_send(void* args){
    int id = ((struct arguments*)args)-> id;
    int sockfd = ((struct arguments*)args) -> sockfd;
    struct sockaddr_in servaddr = ((struct arguments*)args)->servaddr;
    int h_interval = ((struct arguments*)args) -> interval;
    while(1){
        sleep(h_interval);
        string hello;
        hello = "Hello " + to_string(id);
        char* buf = (char*)hello.c_str();
        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        int len = sizeof(cliaddr);
        for(int i = 0;i < neighbors.size(); i++){
            cliaddr.sin_family    = AF_INET; // IPv4
            cliaddr.sin_addr.s_addr = INADDR_ANY;
            cliaddr.sin_port = htons(10000+neighbors[i]);
            pthread_mutex_lock(&lock1);
            sendto(sockfd, (char *)buf, strlen(buf), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
            pthread_mutex_unlock(&lock1);
        }
    }
}
void* hello_receiver(void* args){
    int id = ((struct arguments*)args)-> id;
    int sockfd = ((struct arguments*)args) -> sockfd;
    struct sockaddr_in servaddr = ((struct arguments*)args)->servaddr;
    int h_interval = ((struct arguments*)args) -> interval;
    while(1){
        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        int len = sizeof(cliaddr);
        char buffer[MAXLINE];
        memset(&buffer, 0, MAXLINE);
        recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,(socklen_t*)&len);
        //cout << buffer << endl;
        if(strstr(buffer,"HELLOREPLY")){
            vector<string> tokens;
            stringstream tokenize(buffer);
            string temp;
            while(getline(tokenize,temp,' ')) tokens.push_back(temp);
            bool isthere = false;
            pthread_mutex_lock(&lock2);
            int i;
            for(i = 0; i < M && graph[i][0]!=-1; i++){
                if(graph[i][1] == stoi(tokens[1]) && graph[i][2] == stoi(tokens[2])){
                    graph[i][3] = stoi(tokens[3]);
                    isthere = true;
                }
                else if (graph[i][1] == stoi(tokens[2]) && graph[i][2] == stoi(tokens[1])){
                    graph[i][3] = stoi(tokens[3]);
                    isthere = true;
                }
            }
            //cout << "hi" << i <<endl; 
            if(!isthere && i < M){
                graph[i][0] = 1;
                graph[i][1] = stoi(tokens[1]);
                graph[i][2] = stoi(tokens[2]);
                graph[i][3] = stoi(tokens[3]);
                graphsize++;
            }
            pthread_mutex_unlock(&lock2);
            // cout << "After change / insertion " << graphsize << endl;
            // for(int i = 0; i < M; i++){
            //     cout << graph[i][0] << " " << graph[i][1] << " " << graph[i][2] << " " << graph[i][3] << endl;
            // }

        }
        else if(strstr(buffer,"Hello")){

            vector<string> tokens;
            stringstream tokenize(buffer);
            string temp;
            while(getline(tokenize,temp,' ')) tokens.push_back(temp);
            int j = stoi(tokens[1]);
            string helloreply = "HELLOREPLY ";
            const char* buff;
            //cout << id << " " << j << endl;
            for(int i = 0; i < v.size();i++){
                if((v[i][0] == id && v[i][1] == j)||(v[i][0]==j && v[i][1] == id)){
                    int h = v[i][2] + ( rand() % ( v[i][3] - v[i][2] + 1 ) );
                    // int h; 
                    // if(v[i][3] - v[i][2] > 0){
                    //     h = v[i][2] + ( rand() % ( v[i][3] - v[i][2] + 1 ) );
                    // }
                    string k = to_string(h);
                    helloreply = helloreply + to_string(id) + " " + to_string(j) + " " + k;
                    buff = helloreply.c_str();
                    break;
                }
            }
            pthread_mutex_lock(&lock1);
            sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
            pthread_mutex_unlock(&lock1);
        }
        else if(strstr(buffer,"LSA")){
            vector<string> tokens;
            stringstream tokenize(buffer);
            string temp;
            while(getline(tokenize,temp,' ')) tokens.push_back(temp);
            if(stoi(tokens[3]) == 0) continue;
            if(known_seq_no[stoi(tokens[1])] < stoi(tokens[2])){
                // cout << buffer << endl;
                for(int i = 0,k = 4; i < stoi(tokens[3]);i++,k++,k++){
                    bool isthere = false;
                    int j = 0;
                    pthread_mutex_lock(&lock2);
                    for(j = 0; j < M; j++){
                        if(graph[j][0] == -1)
                            break;
                        if(graph[j][1] == stoi(tokens[1]) && graph[j][2] == stoi(tokens[k])){
                            graph[j][3] = stoi(tokens[k+1]);
                            isthere = true;
                        }
                        else if (graph[j][1] == stoi(tokens[k]) && graph[j][2] == stoi(tokens[1])){
                            graph[j][3] = stoi(tokens[k+1]);
                            isthere = true;
                        }
                    }
                    if(!isthere && j!=M){
                        graph[j][0] = 1;
                        graph[j][1] = stoi(tokens[1]);
                        graph[j][2] = stoi(tokens[k]);
                        graph[j][3] = stoi(tokens[k+1]);
                    }
                    for(int i = 0; i < M; i++){
                        //cout << graph[i][0] << " " << graph[i][1] << " " << graph[i][2] << " " << graph[i][3] << endl;
                    }
                    pthread_mutex_unlock(&lock2);
                    //cout << "LSA end\n";
                }
                for(int j = 0; j < neighbors.size();j++){
                    if(neighbors[j]!=stoi(tokens[1])){
                        pthread_mutex_lock(&lock2);
                        string lsa = "LSA " + to_string(id) + " " + to_string(seq_no) + " " + to_string(graphsize);
                        for(int i = 0; i < graph.size() ; i++){
                            if(graph[i][1] == id){
                                lsa = lsa + " " + to_string(graph[i][2]) + " " + to_string(graph[i][3]) ;
                            }
                            else if(graph[i][2] == id){
                                lsa = lsa + " " +to_string(graph[i][1]) + " " + to_string(graph[i][3]) ;
                            }
                        }
                        pthread_mutex_unlock(&lock2);
                        const char* bu = lsa.c_str();
                        cliaddr.sin_family    = AF_INET; // IPv4
                        cliaddr.sin_addr.s_addr = INADDR_ANY;
                        cliaddr.sin_port = htons(10000+neighbors[j]);
                        pthread_mutex_lock(&lock1);
                        sendto(sockfd, (const char *)bu, strlen(bu), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
                        pthread_mutex_unlock(&lock1);
                    }
                }
                //cout << "LSA number " << number << endl;
                number++;
                known_seq_no[stoi(tokens[1])] = stoi(tokens[2]);
            }
        }
        // for(int i = 0; i < M; i++){
        //     cout << graph[i][0] << " " << graph[i][1] << " " << graph[i][2] << " " << graph[i][3] << endl;
        // }
    }
}
void* lsa_send(void* args){
    int id = ((struct arguments*)args)-> id;
    int sockfd = ((struct arguments*)args) -> sockfd;
    struct sockaddr_in servaddr = ((struct arguments*)args)->servaddr;
    int lsa_interval = ((struct arguments*)args) -> interval;
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    int len = sizeof(cliaddr);
    while(1){
        sleep(lsa_interval);
        if(graphsize == 0) continue;
        pthread_mutex_lock(&lock2);
        string lsa = "LSA " + to_string(id) + " " + to_string(seq_no) + " " + to_string(graphsize);
        //cout << graphsize << endl;
        for(int i = 0; i < graph.size(); i++){
            //cout << id << " " << graph[i][1] << " " << graph[i][2] <<" " << graph[i][3]<< endl;
            if(graph[i][1] == id){
                lsa = lsa + " " + to_string(graph[i][2]) + " " + to_string(graph[i][3]) ;
            }
            else if(graph[i][2] == id){
                lsa = lsa + " " +to_string(graph[i][1]) + " " + to_string(graph[i][3]) ;
            }
        }
        pthread_mutex_unlock(&lock2);
        //cout << lsa << endl;
        const char* bu = lsa.c_str();
        for(int i = 0;i < neighbors.size(); i++){
            cliaddr.sin_family    = AF_INET; // IPv4
            cliaddr.sin_addr.s_addr = INADDR_ANY;
            cliaddr.sin_port = htons(10000+neighbors[i]);
            pthread_mutex_lock(&lock1);
            sendto(sockfd, (const char *)bu, strlen(bu), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
            pthread_mutex_unlock(&lock1);
        } 
        seq_no++;
        //cout << lsa_interval << endl;
    }
}
void dijkstrafromedges(int source,int V,vector< pair<string,int> > &paths){
    int edgecost[V][V];
    for(int i=0;i<V;i++){
        for(int j=0;j<V;j++){
            edgecost[i][j]=0;
        }
    }
    int num = 0;
    for(auto it=graph.begin();it<graph.end();it++){
        if((*it)[0]==-1) break;
        int u = (*it)[1];
        int v = (*it)[2];
        int cost = (*it)[3];
        //cout << u << " " << v <<" " <<cost << endl;
        edgecost[u][v]=cost;
        edgecost[v][u]=cost;
        num ++;
    }
    //cout << V << " " << num << endl;
    int distance[V];
    bool found[V];
    string path[V];
    
    for (int i = 0; i < V; i++){
        path[i] = "";
        distance[i] = INT_MAX;
        found[i] = false;
    }
  
    distance[source] = 0;
    path[source] = to_string(source);

    for (int count = 0; count < V ; count++) {
        int min_distance = INT_MAX, min_node;
  
        for (int v = 0; v < V; v++){
            if (!found[v] && distance[v] <= min_distance){
                min_distance = distance[v];
                //cout << "min_node" << v << endl; 
                min_node = v;
            }
        }
      
        int u = min_node;
        found[u] = true;

        for (int v = 0; v < V; v++){
            if (!found[v] && edgecost[u][v] != 0 && distance[u] != INT_MAX && distance[u] + edgecost[u][v] < distance[v]){
                distance[v] = distance[u] + edgecost[u][v];
                path[v] = path[u] + to_string(v);
            }
        }
    }
    
    for (int i = 0; i < V; i++){
        paths.push_back(make_pair(path[i],distance[i]));
    }
}
void* spf(void* args){
    int id = ((struct arguments*)args)-> id;
    char* out = strdup(((struct arguments*)args) -> filename); 
    int spf_interval = ((struct arguments*)args) -> interval;
    outfile.open(out);
    cout<<out<<endl;
    int time = 0;
    while(1){
        sleep(spf_interval);
        time+=spf_interval;
        vector< pair<string,int> > paths;
        dijkstrafromedges(id,N,paths);
        outfile<<"Routing Table for Node no."<<id<<" at time "<<time<<endl;
        outfile<<"Destination   \tPath   \tCost\n";
        for (int i = 0; i < N; i++){
            if(id!=i){
                outfile<<i<<" \t\t "<<paths[i].first<<" \t\t "<<paths[i].second<<"\n";
            }
        }
    }
}
int taking_input(char* infile){
    cout << "Input reading started\n";
    file.open(infile);
    file >> N >> M;
    string line;
    getline(file,line);
    v.resize(M);
    graph.resize(M);
    for(int i = 0;i<M;i++){
        graph[i].resize(4,-1);
    }
    for(int i = 0;i<M;i++){
        getline(file,line);
        vector<string> tokens;
        stringstream tokenize(line);
        string temp;
        while(getline(tokenize,temp,'(')) tokens.push_back(temp);
        stringstream tokenize1(tokens[1]);
        vector<string> tokens1;
        while(getline(tokenize1,temp,')')) tokens1.push_back(temp);
        stringstream ss(tokens1[0]);
        while(getline(ss,temp,',')) v[i].push_back(stoi(temp));
    }
    cout << "Input read successfully\n";
    return 0;
}
int main(int argc , char *argv[])  
{    
    int id,h_interval = 1,lsa_interval = 5,spf_interval = 20;
    char* infile,*outfile;  
    bool igiven=false,fgiven=false,filegiven = false;
    for(int j = 1; j < argc-1;){
        switch(argv[j++][1]){
            case('i'): id = stoi(argv[j]);igiven = true;break;
            case('h'): h_interval = stoi(argv[j]);break;
            case('a'): lsa_interval = stoi(argv[j]);break;
            case('f'): infile = (argv[j]);fgiven = true;break;
            case('s'): spf_interval = stoi(argv[j]);break;
            case('o'): outfile = argv[j];filegiven = true;break;
        }
    }
    if(!fgiven || !filegiven || !igiven ){
        cout << "Input Error" << endl;
        exit(0);
    }
    string id_str = to_string(id);
    strcat(outfile,"-");
    strcat(outfile,id_str.c_str());
    taking_input(infile);
    for(int i = 0; i< v.size();i++){
        if(v[i][0] == id){
            neighbors.push_back(v[i][1]);
        }
        else if(v[i][1] == id){
            neighbors.push_back(v[i][0]);
        }
    }
    known_seq_no.resize(N);
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
      
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(10000+id);
      
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    int len, n;
  
    len = sizeof(cliaddr);  //len is value/result
    struct arguments* args = (struct arguments *)malloc(sizeof(struct arguments));
    args -> id = id;
    args -> sockfd = sockfd;
    args -> servaddr = servaddr;
    args -> interval = h_interval;
    pthread_t ptid1,ptid2,ptid3,ptid4;
    pthread_create(&ptid1, NULL, &hello_send,(void*)args);
    pthread_create(&ptid2, NULL, &hello_receiver, (void*)args);
    args -> interval = lsa_interval;
    pthread_create(&ptid3, NULL, &lsa_send,(void*)args);
    args -> interval = spf_interval;
    args -> filename = strdup(outfile);
    pthread_create(&ptid4, NULL, &spf, (void*)args);
    while(1);
    // sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
    // cout << "Hello message sent.\n"; 
    return 0;  
}  