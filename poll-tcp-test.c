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
int (*fds_func[10])(int fd, short revents);
typedef int (*fds_callback)(int fd, short revents);
void init_pollfds()
{
	int i;
	for(i=0;i<10;i++){
		fds[i].fd=-1;
		fds_func[i]=NULL;
	}
}
void add_to_fds(int fd, fds_callback cb)
{
	int i=0;
	for(i=1;i<10;i++){
		if(fds[i].fd<0)
			break;
	}
	fds[i].fd=fd;
	fds[i].events=POLLIN|POLLHUP;		
	fds_func[i]=cb;
}
void remove_to_fds(int fd)
{
	int i=0;
	for(;i<10;i++){
		if(fds[i].fd==-1)
			break;
	}
	fds[i].fd=-1;
	fds[i].events=0;
	fds[i].revents=0;
	fds_func[i]=NULL;
}
void dispatch_fds()
{
	int i=0;
	for(i=0;i<10;i++){
        	if(fds[i].revents & (POLLIN|POLLHUP)){
                        fds_func[i](fds[i].fd,fds[i].revents);
                }
        }
}
int tcp_request_callback(int fd,short revents)
{
        char buf[20];
        bzero(buf,20);
        if(revents & POLLHUP){
                printf("tcp connect close\n");
                close(fd);
                remove_to_fds(fd);
                return 0;
        }
        if(!(revents & POLLIN))
                return -1;
        int len=recv(fd,buf,20,0);
        if(len<=0){
                printf(" read error or connect lose\n");
                close(fd);
		remove_to_fds(fd);
        }
        printf("%s\n",buf);
        return 0;
}
int tcp_accept_callback(int fd,short revents)
{
	struct sockaddr cliaddr;
	int clilen=sizeof(cliaddr);
        int connfd=accept(fd,(struct sockaddr*)&cliaddr,&clilen);
        if(connfd>0){
		printf("accept\n");
                add_to_fds(connfd,tcp_request_callback);
        }
        else
                printf("accept error\n");
	return 0;
}
int main(int argc,char * argv[])
{
	int fd,i;
	init_pollfds();
	fd=create_tcp_server("192.168.17.128",atoi(argv[1]));
	//fds[0].fd=fd;
	//fds[0].events=POLLIN|POLLHUP;
	//fds_func[0]=tcp_accept_callback;
	add_to_fds(fd,tcp_accept_callback);
	while(1){
		int ret=poll(fds,10,-1);
		if(ret<1){	
			continue;
			printf("ret is %d\n",ret);
		}
		dispatch_fds();
	}
}
