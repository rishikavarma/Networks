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
void userfunc(int sockfd,char* username){
	char buff[MAX];
	int n;
	for(;;){
		printf("Sub-Prompt-%s> ",username);
		bzero(buff,sizeof(buff));
		fgets(buff,sizeof(buff),stdin);
		char buff1[MAX],buff2[MAX];
		bzero(buff1,sizeof(buff1));
		bzero(buff2,sizeof(buff2));
		int i=0;
		while(buff[i]!='\0'&&buff[i]!='\n'&&buff[i]!='\t'&&buff[i]!=' '){
			buff1[i]=buff[i];
			i++;
		}
		buff1[i]='\0';
		i++;
		if(buff[i]!='\0'&&buff[i]!='\n'){
			while(buff[i]==' '||buff[i]=='\t')i++;
			int r=0;
			while(buff[i]!='\0'&&buff[i]!='\n'){
				buff2[r++]=buff[i++];
			}
			buff2[r]='\0';
		}
		if(!strcmp(buff1,"Readmailnum")){
			if(buff2[0]=='\0') {
				printf("Incorrect command.\n");
				continue;
			}
			char te[100]="READN ";			
			strcat(te,buff2);
			write(sockfd,te,sizeof(te));
			bzero(buff1,sizeof(buff1));
			bzero(buff2,sizeof(buff2));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			if(strcmp(buff,"No more mail.\n")&&strcmp(buff,"Invalid index.\n")){
				while(strcmp(buff,"###\n")){
					printf("%s",buff);
					write(sockfd,"Junk",sizeof("Junk"));
					bzero(buff,sizeof(buff));
					read(sockfd,buff,sizeof(buff));

				}
			}
			else{
				printf("%s",buff);
			}
						
		}

		else if(!strcmp(buff1,"Read")){
			write(sockfd,"READM",sizeof("READM"));
			bzero(buff1,sizeof(buff1));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			if(strcmp(buff,"No more mail.\n")){
				while(strcmp(buff,"###\n")){
					printf("%s",buff);
					write(sockfd,"Junk",sizeof("Junk"));
					bzero(buff,sizeof(buff));
					read(sockfd,buff,sizeof(buff));

				}
			}
			else{
				printf("%s",buff);
			}
						
		}
		else if(!strcmp(buff1,"Deletemailnum")){
			if(buff2[0]=='\0') {
				printf("Incorrect command.\n");
				continue;
			}
			char te[100]="DELN ";			
			strcat(te,buff2);
			write(sockfd,te,sizeof(te));
			bzero(buff,sizeof(buff));
			bzero(buff2,sizeof(buff2));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			printf("%s",buff);
		}
		else if(!strcmp(buff1,"Delete")){
			write(sockfd,"DELM",sizeof("DELM"));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			printf("%s",buff);
		}
		else if(!strcmp(buff1,"Send")){
			if(buff2[0]=='\0') {
				printf("Incorrect command.\n");
				continue;
			}
			char te[100]="SEND ";
			strcat(te,buff2);
			char temp[MAX],fint[MAX],ans[MAX]="",junk[MAX];
			bzero(temp,sizeof(temp));
			write(sockfd,te,sizeof(te));
			read(sockfd,junk,sizeof(junk));
			if(!strcmp(junk,"User doesnot exist.\n")){
				printf("%s",junk);
				continue;
			}
			printf("Type Message: ");
			while(fgets(temp,sizeof(temp),stdin)){
				int i=0,d=0;
				while(temp[i]!='\0'){
					if(temp[i]=='#'&&temp[i+1]=='#'&&temp[i+2]=='#'){
						if(i!=0){
							temp[i]='\n';
							i++;
						}
						d=1;
						break;
					}
					fint[i]=temp[i];
					i++;
				}
				fint[i]='\0';
				if(fint&&strcmp(fint,"\n")){
					write(sockfd,fint,sizeof(fint));
					read(sockfd,junk,sizeof(junk));
				}
				
				if(d==1)break;
				bzero(fint,sizeof(fint));
				bzero(temp,sizeof(temp));
			}
			write(sockfd,"###\n",sizeof("###\n"));
			bzero(buff,sizeof(buff));
			bzero(ans,sizeof(ans));
			bzero(te,sizeof(te));
			bzero(fint,sizeof(fint));
			read(sockfd,buff,sizeof(buff));
			printf("%s",buff);
		}
		
		else if(!strcmp(buff1,"Done")){
			write(sockfd,"DONEU",100);
			return;
		}
		else{
			printf("Incorrect command.\n");
		}

	}
}
void func(int sockfd){
	char buff[MAX];
	int n;
	for(;;){
		printf("Main-Prompt> ");
		bzero(buff,sizeof(buff));
		fgets(buff,sizeof(buff),stdin);
		char buff1[MAX],buff2[MAX];
		int i=0;
		while(buff[i]!='\0'&&buff[i]!='\n'&&buff[i]!='\t'&&buff[i]!=' '){
			buff1[i]=buff[i];
			i++;
		}
		buff1[i]='\0';
		i++;
		if(buff[i]!='\0'&&buff[i]!='\n'){
			while(buff[i]==' '||buff[i]=='\t')i++;
			int r=0;
			while(buff[i]!='\0'&&buff[i]!='\n'){
				buff2[r++]=buff[i++];
			}
			buff2[r]='\0';
		}
		if(!strcmp(buff1,"Listusers")){
			write(sockfd,"LSTU",sizeof("LSTU"));
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			printf("%s\n",buff);
		}
		else if(!strcmp(buff1,"Adduser")){
			if(buff2[0]=='\0') {
				printf("Incorrect command.\n");
				continue;
			}
			char te[100]="ADDU ";			
			strcat(te,buff2);
			write(sockfd,te,sizeof(te));
			bzero(te,sizeof(te));			
			bzero(buff,sizeof(buff));
			read(sockfd,buff,sizeof(buff));
			printf("%s",buff);
		}
		else if(!strcmp(buff1,"Setuser")){
			if(buff2[0]=='\0') {
				printf("Incorrect command.\n");
				continue;
			}
			char te[100]="USER ";
			strcat(te,buff2);
			write(sockfd,te,sizeof(te));
			bzero(te,sizeof(te));	
			bzero(buff,sizeof(buff));			
			read(sockfd,buff,sizeof(buff));
			printf("%s",buff);
			if(strcmp(buff,"User doesnot exist.\n"))userfunc(sockfd,buff2);
			bzero(buff2,sizeof(buff2));
		}
		
		else if(!strcmp(buff1,"Quit")){
			write(sockfd,"QUIT",100);
			printf("Client exiting.\n");
			break;
		}
		else{
			printf("Incorrect command.\n");
		}
		bzero(buff1,sizeof(buff1));
		bzero(buff2,sizeof(buff2));
	}
}

int main(int argc, char* argv[]){
	if(argc<2){
		printf("Ip address not given.\n");
		exit(0);
	}
	int sockfd,connfd;
	struct sockaddr_in servaddr,cli;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1){
		printf("Socket creation failed...\n");
		exit(0);
	}
	else{
		printf("Socket succesfully created...\n");
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);
	servaddr.sin_port=htons(PORT);
	if(connect(sockfd,(SA*)&servaddr,sizeof(servaddr))!=0){
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else{
		printf("Connected to the server...\n");
	}
	func(sockfd);
	close(sockfd);
}