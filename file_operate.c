#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int copyfile(char* path,char* newfile)
{
	int fd_old,fd_new;
	char c[1024]={0};
	int readlen=0;
	fd_old=open(path,O_RDONLY);
	if(fd_old<0)
		return -1;
	fd_new=open(newfile,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	if(fd_new<0){
		printf("open fail\n");
		return -1;
	}
	while((readlen=read(fd_old,&c,1024))>0)
		write(fd_new,&c,readlen);
	close(fd_old);
	close(fd_new);
	return 1;
}
int copyfile2(char *path,char* newfile)
{
	FILE *fd_old,*fd_new;
	int readlen;
	char c[1024]={0};
	fd_old=fopen(path,"r");
	fd_new=fopen(newfile,"w");
	while( (readlen=fread(c,1,1024,fd_old))>0)
		fwrite(c,1,readlen,fd_new);
	fclose(fd_old);
        fclose(fd_new);
}
int replace_str(char *str,char *strdest,char *strnew)
{
	char *p=strstr(str,strdest);
	char *p1=str;
	int i=0;
	int len1=strlen(strdest);
	int len2=strlen(strnew);
	if(p!=NULL){
		while(p1!=p){
			*p1=str[i]
			p1++;
			i++;
		}
		for(i=0;i<len2;i++){
			*p1=strnew[i];
			p1++;
		}	
		
		return 1;	
	}
	else
	 return -1;
}
int replace_infile(char* path,char* newfile,char *oldstr,char *newstr)
{
	FILE *fd_old,*fd_new;
	char c[1024]={0};
	int readlen;
	fd_old=fopen(path,"r");
	fd_new=fopen(newfile,"w");
	readlen=fread(c,1,1024,fd_old);
	
}
int main(int argc,char *argv[])
{
	//copyfile2("/home/jjw/ttemp2/1.txt","/home/jjw/ttemp2/2.txt");
	replace_str("1234567890abc123","abc","123");
}
