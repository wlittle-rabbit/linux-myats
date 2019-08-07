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

int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300,
		   B115200, B19200, B9600, B4800, B2400, B1200, B300, B38400,B57600};
int name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300, 115200,  
                  19200,  9600, 4800, 2400, 1200,  300, 38400,57600};
static int set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(speed_arr[0]);  i++) {
		if  (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
		   
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0) {
				perror("tcsetattr fd");
				return -1;
			}
			tcflush(fd,TCIOFLUSH);
			return 0;
		}
	}
	return -1;
}
int open_serial(void)
{
	int fd = -1;
	char *serial = NULL;
	struct termios options = {0};
/*	if ((serial = find_serial()) == NULL) {
		return -1;
	}*/
	fd = open(serial, O_RDWR);
	if (fd < 0) {
	//	LOGE("open %s error: %s \n", serial, strerror(errno));
		return -1;
	}
	set_speed(fd, 115200);
	
	#if 1
	if ( tcgetattr( fd,&options) != 0) {
	//	LOGE("%s:%d tcgetattr fail: %s\n", __FUNCTION__, __LINE__, strerror(errno));
		close(fd);
		fd = -1;
		return -1;
	}
	options.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG); /*Input*/
	options.c_iflag &= ~(BRKINT|ICRNL|INPCK|ISTRIP|IXON);
	options.c_cflag &= ~(CSIZE|PARENB);
	options.c_cflag |= CS8;
	options.c_oflag &= ~OPOST;
	options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 0;
	  
	if (tcsetattr(fd, TCSAFLUSH, &options) != 0) {
		perror("tcsetattr fail");
		close(fd);
		fd = -1;
	}
	#endif
	return fd;
}
int tcp_connect_to(char *remote_ip,int remote_port)
{
	int sockfd=-1;
	struct sockaddr_in serv_addr;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1){
		printf("socket eeror\n");
		return -1;
	}
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(remote_port);
	serv_addr.sin_addr.s_addr=inet_addr(remote_ip);
	
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr))==-1){
		printf("connect error\n");
		return -1;
	}
	return sockfd;
}
void tcpclenttask(int fd)
{
	while(1){
		write(fd,"123456",6);
		sleep(2);
	}
}
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
/*短连接
void tcpservertask(int fd)
{
	char buf[20]={0};
	struct sockaddr clientaddr;
	int len=sizeof(clientaddr);
	while(1){
		int clientfd=accept(fd,&clientaddr,&len);
		if(read(clientfd,buf,20)<0){
			printf("lose connect\n");
			break;
		}
		printf("%s",buf);
		sleep(1);
		printf("wait 1s\n");
		close(clientfd);
	}
}*/
void tcpservertask(int fd)
{
	pid_t fpid;
	int fd_tmp;
        char buf[20]={0};
        struct sockaddr clientaddr;
        int len=sizeof(clientaddr);
	while(1){
        	int clientfd=accept(fd,&clientaddr,&len);
		if(clientfd>0){
			fpid=fork();	
			if(fpid<0){
				printf("fork error\n");
			}
			else if(fpid==0){
				//printf("child pid\n");
				fd_tmp=clientfd;
				break;
			}
			else{
				//printf("father pid\n");
				continue;
			}
		}
	}  
        while(1){
		if(recv(fd_tmp,buf,20,0)<=0){
                	printf("lose connect\n");
			close(fd_tmp);
			exit(0);
                        break;
                }
                printf("%s",buf);
                sleep(1);
                printf("wait 1s\n");
	}
}

pthread_t id[10];
void pthread_tcpserver(int fd)
{	
	char buf[20]={0};
	while(1){
		if(recv(fd,buf,20,0)<=0){
			printf("client close socket\n");
			break;
		}
		printf("%s\n",buf);
		sleep(1);
		printf("thread wait 1s\n");
	}
	close(fd);
	int *p=(int*)malloc(sizeof(int));
	*p=1;
	pthread_exit((void *)p);
}
void tcpservertask_thread(int fd)
{
        pid_t fpid;
        int fd_tmp;
	int i=0;
        struct sockaddr clientaddr;
        int len=sizeof(clientaddr);
        while(1){
                int clientfd=accept(fd,&clientaddr,&len);
		if(clientfd>0){
			fd_tmp=clientfd;
			pthread_create(&id[i],NULL,(void *)pthread_tcpserver,fd_tmp);
			i++;
		}
	}
}


#define OPENMAX 11
struct pollfd fd_clients[OPENMAX];
void tcpservertask_poll(int fd)
{
	int i=0;
	struct sockaddr clientaddr;
	int clientlen=sizeof(clientaddr);
	for(i=0;i<OPENMAX;i++)
		fd_clients[i].fd=-1;
	
	fd_clients[0].fd=fd;
	fd_clients[0].events=POLLIN;
	while(1){
		int ret=poll(fd_clients,OPENMAX,-1);
		//printf("ret is %d\n",ret);
		if(fd_clients[0].revents & POLLIN)
		{
			printf("accept client\n");
			int connfd=accept(fd_clients[0].fd,(struct sockaddr*)&clientaddr,&clientlen);
			if(connfd>0){
				for(i=1;i<OPENMAX;i++){
					if(fd_clients[i].fd<0){
						fd_clients[i].fd=connfd;
						fd_clients[i].events=POLLIN;
						break;
					}
				}
			}
			else
				printf("accept error\n");
				
		}
		for(i=1;i<OPENMAX;i++)
		{	
			if(fd_clients[i].revents & POLLIN){
				int len=0;
				char buf[20]={0};
				len=recv(fd_clients[i].fd,buf,20,0);
				if(len<=0){
					printf("client lose\n");
					close(fd_clients[i].fd);
					fd_clients[i].fd=-1;
				}
				else{
					printf("%s\n",buf);
				}
				if(ret<=1)
					break;
			}
			
		}
	}
}


#define ATS_SOCKET "/tmp/ats_socket"
int create_server_socket()
{
	int sockfd;
	struct sockaddr_un myaddr;
	unlink(ATS_SOCKET);
	sockfd=socket(AF_UNIX,SOCK_STREAM,0);
	myaddr.sun_family=AF_UNIX;
	strncpy(myaddr.sun_path,ATS_SOCKET,sizeof(myaddr.sun_path)-1);
	bind(sockfd,(struct sockaddr*)&myaddr,sizeof(myaddr));
	listen(sockfd,1);
	chmod(ATS_SOCKET,0777);
	return sockfd;
}

int main(int argc,char *argv[])
{
	/*int clientfd;
	clientfd=tcp_connect_to("192.168.17.1",20000);
	tcpclenttask(clientfd);
	close(clientfd);*/
/*	int serverfd;
	serverfd=create_tcp_server("192.168.17.128",20001);
	tcpservertask(serverfd);
	close(serverfd);*/
	/*int serverfd;
        serverfd=create_tcp_server("192.168.17.128",20001);
        tcpservertask_thread(serverfd);
        close(serverfd);*/
	/*int serverfd;
        serverfd=create_tcp_server("192.168.17.128",atoi(argv[1]));
        tcpservertask_poll(serverfd);
        close(serverfd);*/

	//struct pollfd fds[10];
	
	char buff[20]={0};
	int sockfd=create_server_socket();
	struct sockaddr_un remaddr;
	int clientfd;
	int clilen=sizeof(remaddr);
	while(1){
		clientfd=accept(sockfd,(struct sockaddr*)&remaddr,&clilen);
		printf("accept socket\n");
		while(1){
			int len=read(clientfd,buff,20);
			 printf("socket get %s",buff);
			write(clientfd,"hello",5);
			if(len<=0){
				printf("socket lose\n");
				close(clientfd);
				break;
			}
		}
	}
	return 1;
}
