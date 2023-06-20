#include <stdio.h>
#include <fcntl.h>
#include <string.h>
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
#define MAX 10000
#define PORT 8080
#define SA struct sockaddr

char currentuser[MAX];
FILE *currentuserfile=NULL;
char* users[MAX];
int usenum=0,num_of_msgs=0,curr_msg=0;
FILE *full_list=NULL;
void list_of_users(){
	full_list=fopen("full_list.txt","a");
	fclose(full_list);
	full_list=fopen("full_list.txt","r");
	rewind(full_list);
	int i=0;
	char buff[MAX];	
	while(fgets(buff,sizeof(buff),full_list)){
		users[i]=(char*)malloc(sizeof(buff));
		char* tq=users[i];
		int j=0;
		while(buff[j]!='\0'&&buff[j]!='\n'){
			*tq=buff[j];
			tq++;
			j++;
		}
		i++;
		bzero(buff,MAX);
	}
	usenum=i;
	fclose(full_list);
	full_list=fopen("full_list.txt","a");
	return;
}



int add_user(char* a){
	for(int i=0;i<usenum;i++){
		char* n=users[i];
		if(!strcmp(n,a)){
			return -1;
		}
	}
	users[usenum]=(char*)malloc(sizeof(*a));
	strcpy(users[usenum],a);
	fprintf(full_list,"%s\n",a);
	usenum++;
	char ts1[MAX];
	strcpy(ts1,a);
	strcat(ts1,"_spool.txt");
	FILE *fp=fopen(ts1,"w");
	fclose(fp);
	return 0;
}

int open_user(char* a){
	int n=0;
	for(int i=0;i<usenum;i++){
		char* l=users[i];
		if(!strcmp(l,a)){
			strcpy(currentuser,a);
			char ts1[MAX];
			strcpy(ts1,a);
			strcat(ts1,"_spool.txt");
			FILE* ptr=fopen(ts1,"r");
			currentuserfile=ptr;
			char buff[MAX];
			while(fgets(buff,sizeof(buff),ptr)){
				if(!strcmp(buff,"###\n"))n++;
				bzero(buff,MAX);
			}
			rewind(currentuserfile);
			num_of_msgs=n;
			return n;
		}
	}
	return -1;

}
int read_mail(int sockfd){
	if(num_of_msgs==0){
		write(sockfd,"No more mail.\n",sizeof("No more mail.\n"));
		return -1;
	}
	if(curr_msg==num_of_msgs){
		rewind(currentuserfile);
		curr_msg=0;
	}
	char ans[MAX];
	bzero(ans,MAX);
	fgets(ans,sizeof(ans),currentuserfile);
	while(strcmp(ans,"###\n")){
		write(sockfd,ans,sizeof(ans));
		read(sockfd,ans,sizeof(ans));
		bzero(ans,MAX);
		fgets(ans,sizeof(ans),currentuserfile);
	}
	write(sockfd,ans,sizeof(ans));	
	curr_msg++;
	return 0;
}
int read_mail_num(int sockfd,int num){
	if(num_of_msgs==0){
		write(sockfd,"No more mail.\n",sizeof("No more mail.\n"));
		return -1;
	}
	if(num>num_of_msgs||num<=0){
		write(sockfd,"Invalid index.\n",sizeof("Invalid index.\n"));
		return -1;
	}
	if(curr_msg>num-1){
		rewind(currentuserfile);
		curr_msg=0;
	}
	char ans[MAX];
	bzero(ans,MAX);
	while(curr_msg<num-1){
		fgets(ans,sizeof(ans),currentuserfile);
		if(!strcmp(ans,"###\n"))curr_msg++;
	}
	
	fgets(ans,sizeof(ans),currentuserfile);
	while(strcmp(ans,"###\n")){
		write(sockfd,ans,sizeof(ans));
		read(sockfd,ans,sizeof(ans));
		bzero(ans,MAX);
		fgets(ans,sizeof(ans),currentuserfile);
	}
	write(sockfd,ans,sizeof(ans));	
	curr_msg++;
	return 0;
}
int delete_mail(){
	if(num_of_msgs==0)return -1;
	if(curr_msg==num_of_msgs){
		rewind(currentuserfile);
		curr_msg=0;
	}
	FILE* temp=fopen("temp_file.txt","w");
	char ts1[MAX];
	strcpy(ts1,currentuser);
	strcat(ts1,"_spool.txt");
	FILE* ptr=fopen(ts1,"r");
	char buff[MAX];
	for(int i=0;i<num_of_msgs;){
		bzero(buff,MAX);
		fgets(buff,sizeof(buff),ptr);
		if(i!=curr_msg){
			fputs(buff,temp);
		}
		if(!strcmp(buff,"###\n")){
			i++;
		}
	}
	fclose(ptr);
	fclose(temp);
	temp=fopen("temp_file.txt","r");
	ptr=fopen(ts1,"w");
	bzero(buff,MAX);
	while(fgets(buff,sizeof(buff),temp)){
		fputs(buff,ptr);
		bzero(buff,MAX);
	}
	fclose(currentuserfile);
	fclose(ptr);
	fclose(temp);
	currentuserfile=fopen(ts1,"r");
	for(int i=0;i<curr_msg;){
		bzero(buff,MAX);
		fgets(buff,sizeof(buff),currentuserfile);
		if(!strcmp(buff,"###\n")){
			i++;
		}
	}
	
	remove("temp_file.txt");
	num_of_msgs--;
	return 0;
}
int delete_mail_num(int sockfd,int num){
	if(num_of_msgs==0){
		write(sockfd,"No more mail.\n",sizeof("No more mail.\n"));
		return -1;
	}
	if(num>num_of_msgs||num<=0){
		write(sockfd,"Invalid index.\n",sizeof("Invalid index.\n"));
		return -1;
	}
	if(curr_msg>num-1){
		rewind(currentuserfile);
		curr_msg=0;
	}
	char ans[MAX];
	bzero(ans,MAX);
	while(curr_msg<num-1){
		fgets(ans,sizeof(ans),currentuserfile);
		if(!strcmp(ans,"###\n"))curr_msg++;
	}
	FILE* temp=fopen("temp_file.txt","w");
	char ts1[MAX];
	strcpy(ts1,currentuser);
	strcat(ts1,"_spool.txt");
	FILE* ptr=fopen(ts1,"r");
	char buff[MAX];
	for(int i=0;i<num_of_msgs;){
		bzero(buff,MAX);
		fgets(buff,sizeof(buff),ptr);
		if(i!=curr_msg){
			fputs(buff,temp);
		}
		if(!strcmp(buff,"###\n")){
			i++;
		}
	}
	fclose(ptr);
	fclose(temp);
	temp=fopen("temp_file.txt","r");
	ptr=fopen(ts1,"w");
	bzero(buff,MAX);
	while(fgets(buff,sizeof(buff),temp)){
		fputs(buff,ptr);
		bzero(buff,MAX);
	}
	fclose(currentuserfile);
	fclose(ptr);
	fclose(temp);
	currentuserfile=fopen(ts1,"r");
	for(int i=0;i<curr_msg;){
		bzero(buff,MAX);
		fgets(buff,sizeof(buff),currentuserfile);
		if(!strcmp(buff,"###\n")){
			i++;
		}
	}
	
	remove("temp_file.txt");
	num_of_msgs--;
	write(sockfd,"Message deleted.\n",sizeof("Message deleted.\n"));
	return 0;
}


int send_user(char* a,int sockfd){
	for(int i=0;i<usenum;i++){
		char* n=users[i];
		if(!strcmp(n,a)){
			char ts1[MAX],ts2[MAX]="From: ",ts3[MAX]="To: ",ts4[MAX]="Date: IST ",buff[MAX];
			time_t dty;   
    		time(&dty);
			strcpy(ts1,a);
			strcat(ts2,currentuser);
			strcat(ts3,a);
			strcat(ts4,ctime(&dty));
			strcat(ts1,"_spool.txt");
			FILE *sptr=fopen(ts1,"a");
			fputs(ts2,sptr);
			fputs("\n",sptr);
			fputs(ts3,sptr);
			fputs("\n",sptr);
			fputs(ts4,sptr);

			write(sockfd,"Junk",sizeof("Junk"));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			while(strcmp(buff,"###\n")){
				if(buff){
					fputs(buff,sptr);
					fputs("\n",sptr);
				}
				write(sockfd,"Junk",sizeof("Junk"));
				bzero(buff,sizeof(buff));
				read(sockfd,buff,sizeof(buff));
			}
			fprintf(sptr,"\n");
			fprintf(sptr,"%s\n",buff);	
			fclose(sptr);
			if(!strcmp(a,currentuser)){
				num_of_msgs++;
			}
			return 0;
		}
	}
	return -1;
}

void func(int sockfd){
	char buff[MAX];
	int n;
	int user_set=0;
	for(;;){
		bzero(buff,MAX);
		read(sockfd,buff,sizeof(buff));		
		char buff1[MAX],buff2[MAX];
		int i=0;
		bzero(buff1,MAX);
		bzero(buff2,MAX);
		while(buff[i]!='\0'&&buff[i]!='\t'&&buff[i]!=' '){
			buff1[i]=buff[i];
			i++;
		}
		buff1[i]='\0';
		i++;
		if(buff[i]!='\0'){
			while(buff[i]==' '||buff[i]=='\t')i++;
			int r=0;
			while(buff[i]!='\0'){
				buff2[r++]=buff[i++];
			}
			buff2[r]='\0';
		}
		if(!strcmp("LSTU",buff1)){
			char temp[MAX]="";
			bzero(temp,MAX);
			for(int i=0;i<usenum;i++){
				strcat(temp,users[i]);
				strcat(temp," ");
			}
			write(sockfd,temp,sizeof(temp));
		}
		else if(!strcmp("ADDU",buff1)){
			int k=add_user(buff2);
			if(k==-1)write(sockfd,"Userid already present.\n",sizeof("Userid already present.\n"));
			else write(sockfd,"User added.\n",sizeof("User added.\n"));
		}
		else if(!strcmp("USER",buff1)){
			int k=open_user(buff2);
			if(k==-1)write(sockfd,"User doesnot exist.\n",sizeof("User doesnot exist.\n"));
			else{
				user_set=1;
				char temp[MAX]="User exists and has ",temp2[MAX];
				sprintf(temp2, "%d", k);
				strcat(temp,temp2);
				strcat(temp," messages.\n");
				write(sockfd,temp,sizeof(temp));
			}
		}
		else if (!strcmp("READM",buff1)){
			int k=read_mail(sockfd);
			
		}
		else if (!strcmp("READN",buff1)){
			int num=atoi(buff2);
			int k=read_mail_num(sockfd,num);
		}
		else if (!strcmp("DELM",buff1)){
			int k=delete_mail();
			if(k==-1)write(sockfd,"No more mail.\n",sizeof("No more mail.\n"));
			else write(sockfd,"Message deleted.\n",sizeof("Message deleted.\n"));
		}
		else if (!strcmp("DELN",buff1)){
			int num=atoi(buff2);
			int k=delete_mail_num(sockfd,num);
		}
		else if (!strcmp("SEND",buff1)){	
			int ref=0;
			for(int i=0;i<usenum;i++){
				char* n=users[i];
				if(!strcmp(n,buff2)){	
					int k=send_user(buff2,sockfd);
					ref=1;
					break;
				}
			}			
			if(ref==0)write(sockfd,"User doesnot exist.\n",sizeof("User doesnot exist.\n"));
			else write(sockfd,"Mail sent.\n",sizeof("Mail sent.\n"));
		}
		
		else if (!strcmp("DONEU",buff)){
			bzero(currentuser,MAX);
			currentuserfile=NULL;
			curr_msg=0;
			num_of_msgs=0;
		}
		else if (!strcmp("QUIT",buff)){
			printf("Server exiting.\n");
			break;
		}
	}
}


int main(){
	list_of_users();

	
	int sockfd,connfd,len;
	struct sockaddr_in servaddr,cli;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1){
		printf("Socket creation failed.\n");
		exit(0);
	}
	else{
		printf("Socket succesfully created.\n");
	}
	bzero(&servaddr,sizeof(servaddr));

	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(PORT);

	if((bind(sockfd, (SA*)&servaddr,sizeof(servaddr)))!=0){
		printf("Socket bind failed.\n");
		exit(0);
	}
	else{
		printf("Server listening.\n");
	}

	if((listen(sockfd,5))!=0){
		printf("Listen failed.\n");
		exit(0);
	}
	else{
		printf("Server listening.\n");
	}
	len=sizeof(cli);

	connfd=accept(sockfd,(SA*)&cli,&len);
	if(connfd<0){
		printf("Server accept failed.\n");
		exit(0);
	}
	else{
		printf("Server accepted the client.\n");
	}

	func(connfd);
	close(sockfd);
	fclose(full_list);
}

