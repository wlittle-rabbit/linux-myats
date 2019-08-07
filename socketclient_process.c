
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <assert.h>
#include <pthread.h>
#include <sys/wait.h>
#include <termios.h>
#include <poll.h>
#include <time.h>
#include <sys/un.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <net/if.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h> 

#define LOGE	printf
#define ATS_SOCKET "/tmp/ats_socket"

static int connect_timeout(int fd, int timeout, struct sockaddr* addr, int len)
{
	struct pollfd pollfds[1];
	int r = 0;
	fcntl(fd, F_SETFL, O_NONBLOCK);
	pollfds[0].fd = fd;
        pollfds[0].events = POLLOUT;
	r = poll(pollfds, 1, timeout);
        if (r == 1 && (pollfds[0].revents & POLLOUT) != 0) {
	  return connect(fd, addr, len);
        }
	return -1;
}
static int response(int fd, int timeout, char *buf, int len)
{
	struct pollfd pollfds[1];
	int r = 0;
	pollfds[0].fd = fd;
        pollfds[0].events = POLLIN;
	r = poll(pollfds, 1, timeout);
	if (pollfds[0].revents & POLLHUP) {
		return -2;
	} else if (r == 1 && (pollfds[0].revents & POLLIN) != 0) {
		return read(fd, buf, len);
        } 
	return -1;
}
int send_command(char *cmd, char *result, int len, int timeout)
{
	int ret = -1;
	int at_server_socket=init_at_socket();
	if (at_server_socket < 0) {
		
		return -1;
	}
	//snprintf(tmp, sizeof tmp, "atcmd:%s", cmd);
	write(at_server_socket, cmd, strlen(cmd));
	ret = response(at_server_socket, timeout, result, len);
	close(at_server_socket);
	if (ret <= 0) {
		LOGE("fail or timeout\n ");
		return -1;	
	}
	result[len-1]=0;
	return 0;
}
int init_at_socket(void)
{
	int connfd=-1;
	struct sockaddr_un srvaddr;
	connfd=socket(AF_UNIX,SOCK_STREAM,0);
	srvaddr.sun_family=AF_UNIX;
	strcpy(srvaddr.sun_path,ATS_SOCKET);
	connect_timeout(connfd,3000,(struct sockaddr*)&srvaddr,sizeof(srvaddr));
	return connfd;
}

int main()
{
	char result[20];
	int connfd;
	int ret=send_command("hello",result,20,3000);
	if(ret==0)
		printf("result is %s\n",result);
}
