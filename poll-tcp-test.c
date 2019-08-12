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

#define CBUF_LEN 100
struct cycle_buffer{
	unsigned int in;
	unsigned int out;//必须为unsigned int 型，环形计数
	unsigned int len;
	unsigned char buf[CBUF_LEN];
};
struct cycle_buffer mybuff;
int min(int a,int b)
{
	return a<=b?a:b;
}
void init_cbuff(struct cycle_buffer * cbuf)
{
	cbuf->in=0;
	cbuf->out=0;
	cbuf->len=CBUF_LEN;
}
/*``````````````*/
/* 需要考虑存放数据长度是否大于可用空间
    数据是否需要分两端保存
	上次刚好写到末尾*/
/*```````````````*/
int put_to_cbuff(struct cycle_buffer *cbuf,char *buf,int len)
{
	int l;
	if(len>(CBUF_LEN-cbuf->in+cbuf->out))
	{
		printf("put the data length more than buffer's valid len\n");
		len=min(len,CBUF_LEN-cbuf->in+cbuf->out);
	}
	l=min(len,CBUF_LEN-cbuf->in);
	memcpy(cbuf->buf+(cbuf->in),buf,l);//buffer一般是2的n次方，要处理数据写到末尾情况
	memcpy(cbuf->buf,buf+l,len-l);//切换到头部了
	cbuf->in=cbuf->in+len;
	return len;
}
int get_from_cbuff(struct cycle_buffer *cbuf,char *buf,int len)
{
	int l;
	if(len>(cbuf->in-cbuf->out))
        {
                printf("get the data length more than buffer's valid len\n");
                len=min(len,cbuf->in-cbuf->out);
        }
	l=min(len,CBUF_LEN-cbuf->out);
	memcpy(buf,cbuf->buf,l);
	memcpy(buf+l,cbuf->buf,len-l);//第二段
	cbuf->out=cbuf->out+len;
	return len;
}

int hello(int fd,char *buf,int len)
{
	printf("cmd:%s\n",buf);
	write(fd,"hello",5);
}
int mynetread(int fd,char *buf,int len)
{
	char buff[100]={0};
//	printf("MYNETREAD:%s\n",buf);
	get_from_cbuff(&mybuff,buff,len);
	printf("%s\n",buff);
}
int mynetwrite(int fd,char *buf,int len)
{
//	printf("MYNETWRITE:%s\n",buf);
	char temp[20]="a23456789";
	char buff[20]={0};
	put_to_cbuff(&mybuff,temp,strlen(temp));
	get_from_cbuff(&mybuff,buff,strlen(temp));
	printf("%s\n",buff);
}
typedef int (*handler)(int fd,char *buf,int len);
struct cmd_handler{
	char *cmd;
	handler handler;
	char *rsp;
};
struct cmd_handler request[]={
	{"hello",hello},
	{"READ",mynetread},
	{"WRITE",mynetwrite},
	{"cmd1",NULL,"this is cmd1"},
	{NULL,NULL},
};


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
	int i=0;
	int readlen=100;
        char readbuf[readlen];
        bzero(readbuf,readlen);
        if(revents & POLLHUP){
                printf("tcp connect close\n");
                close(fd);
                remove_to_fds(fd);
                return 0;
        }
        if(!(revents & POLLIN))
                return -1;
        int len=recv(fd,readbuf,readlen,0);
        if(len<=0){
                printf(" read error or connect lose\n");
                close(fd);
		remove_to_fds(fd);
		return 0;
        }
        printf("recv is %s\n",readbuf);
	struct cmd_handler *ch=NULL;
	ch = request;
        while(ch && ch->cmd && (ch->handler||ch->rsp)){	
		if(!strncmp(ch->cmd,readbuf,strlen(ch->cmd))){
			if(ch->handler)
				ch->handler(fd,readbuf,len);
			else
				write(fd,ch->rsp,strlen(ch->rsp));
			return 0;
		}
		ch++;
	}
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
	init_cbuff(&mybuff);	
	int fd,i;
	init_pollfds();
	fd=create_tcp_server("192.168.17.128",atoi(argv[1]));
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
