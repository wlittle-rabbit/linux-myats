
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <poll.h>

int create_tcp_server(char *server_ip,int port)
{
        int sockfd=-1;
        struct sockaddr_in myaddr;
        sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(sockfd==-1){
                printf("socket error\n");
        }
        myaddr.sin_family=AF_INET;
        myaddr.sin_port=htons(port);
        myaddr.sin_addr.s_addr=inet_addr(server_ip);
        if(bind(sockfd,(struct sockaddr*)&myaddr,sizeof(struct sockaddr))==-1){
                printf("bind error\n");
                return -1;
        }
        if(listen(sockfd,100)==-1){
                printf("listen error\n");
                return -1;
        }
        return sockfd;
}

struct pollfd fds[10];
void init_pollfds()
{
	int i;
	for(i=0;i<10;i++)
		fds[i].fd=-1;
}
void add_to_fds(int fd)
{
	int i=0;
	for(i=1;i<10;i++){
		if(fds[i].fd<0)
			break;
	}
	fds[i].fd=fd;
	fds[i].events=POLLIN;		
}
int main(int argc,char * argv[])
{
	int fd,i;
	int connfd;
	char buf[20]={0};
	struct sockaddr cliaddr;
	int clilen=sizeof(cliaddr);
	init_pollfds();
	fd=create_tcp_server("192.168.17.128",atoi(argv[1]));
	fds[0].fd=fd;
	fds[0].events=POLLIN;
	while(1){
		int ret=poll(fds,10,-1);
		if(ret<1){	
			continue;
			printf("ret is %d\n",ret);
		}
		if(fds[0].revents & POLLIN){
			connfd=accept(fd,(struct sockaddr*)&cliaddr,&clilen);
			if(connfd>0){
				printf("accept\n");
				add_to_fds(connfd);
			}
			else
				printf("accept error\n");
		}
		for(i=1;i<10;i++){	
			if(fds[i].revents & POLLIN){
				printf("ret is %d\n",ret);
				bzero(buf,20);
				int len=recv(fds[i].fd,buf,20,0);
				if(len<=0){
					printf("client close\n");
					close(fds[i].fd);
					fds[i].fd=-1;
				}
				printf("%s\n",buf);
			}
		}
	}
}
