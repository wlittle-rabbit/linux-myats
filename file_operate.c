#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

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
int replace_str_samelen(char *str,char *strdest,char *strnew,int len)
{
	char *p;
	int i=0;
	while((p=strstr(str,strdest))!=NULL){
		for(i=0;i<len;i++){
			*p=strnew[i];
			p++;
		}
	}
		return 1;	
}
char* replace_str_1(char *str,char *strdest,char *strnew)
{
        char *p;
	char *res;
	char *result=res;
	char *temp=str;
        int i=0;
	int len1=strlen(strdest);
	int len2=strlen(strnew);
	printf("%d,%d\n",len1,len2);
        if((p=strstr(str,strdest))!=NULL){
		while(temp!=p){
			*res=*temp;
			res++;
			temp++;
		}
		temp=temp+len1;
		for(i=0;i<len2;i++)
			*res++=strnew[i];
		while(*temp!='\0'){
			*res=*temp;
			temp++;
			res++;
		}
		*res=*temp;
		str=result;
        }
	printf("%s\n",result);
                return result;
}
char* replace_str_multi(char *str,char *strdest,char *strnew)
{
        char *p;
        char *res;
        char *result=res;
        char *temp=str;
        int i=0;
        int len1=strlen(strdest);
        int len2=strlen(strnew);
        printf("%d,%d\n",len1,len2);
        while((p=strstr(temp,strdest))!=NULL){
                while(temp!=p){
                        *res=*temp;
                        res++;
                        temp++;
                }
                temp=temp+len1;
                for(i=0;i<len2;i++)
                        *res++=strnew[i];
                while(*temp!='\0'){
                        *res=*temp;
                        temp++;
                        res++;
                }
                *res=*temp;
		temp=str;
		res=result;
                for(i=0;i<strlen(result);i++){
			temp[i]=res[i];
		}
        }
        printf("%s\n",result);
                return result;
}

int find_dev_name(char *path,char *predev)
{
	struct dirent *de;
	DIR *d=opendir(path);
	if(d==NULL)
		return -1;
	while( (de=readdir(d))){
		if(de->d_type == DT_DIR || de->d_name[0]=='.')//DT_DIR目录类型
			continue;
		if(strstr(de->d_name,predev)!=NULL){
			closedir(d);
			printf("find the dev\n");
			return 0;
		}
	}
	closedir(d);
	return -1;
}
int main(int argc,char *argv[])
{
	printf("%s\n",argv[1]);
	if(find_dev_name(argv[1],"null")==-1)
		printf("not find dev\n");
}
