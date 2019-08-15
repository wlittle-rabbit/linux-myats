#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int copyfile(char* path,char* newfile)
{
	int fd_old,fd_new;
	char c;
	fd_old=open(path,O_RDONLY);
	if(fd_old<0)
		return -1;
	fd_new=open(newfile,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	if(fd_new<0){
		printf("open fail\n");
		return -1;
	}
	while(read(fd_old,&c,1)==1)
		write(fd_new,&c,1);
	return 1;
}
int main(int argc,char *argv[])
{
	copyfile("/home/jjw/ttemp2/1.txt","/home/jjw/ttemp2/2.txt");
}
