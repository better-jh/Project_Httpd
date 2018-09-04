#include"http.h"
int a = 10;
int b = 11;
int c = 12;
int d = 10;
int e = 11;
int f = 12;
int g = 10;
int h = 11;
int i = 12;
bool j = true;
bool k = false;
float m = 0.1;
float n = 0.2;
int startup(const char* ip,int port)
{
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("socket");
		return -1;
	}
	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=htons(port);
	local.sin_addr.s_addr=inet_addr(ip);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
	{
		perror("bind");
		return -2;
	}
	if(listen(sock,10)<0)
	{
		perror("listen");
		return -3;
	}
	return sock;
}

static int get_line(int fd,char* buf,int len)
{
	char c='\0';
	int i=0;
	while(c!='\n' && i<len-1)
	{
		ssize_t s=recv(fd,&c,1,0);
		if(s>0)
		{
			if(c=='\r')// \r \r\n \rc->\n
			{
				recv(fd,&c,1,MSG_PEEK);
				if(c=='\n')
				{
					recv(fd,&c,1,0);
				}
				else
				{
					c = '\n';
				}
			}
			buf[i++]=c;
		}
	}
	buf[i] = 0;
	return i;
}

void drop_header(int fd)
{
	char buff[SIZE];
	int ret=-1;
	do{
		ret=get_line(fd,buff,sizeof(buff));
		printf("%s",buff);
	}while(ret>0 && strcmp(buff,"\n"));
}


void print_log(const char* msg,int level)
{
	const char *level_msg[]={ "NOTICE","WARNING","FATAL"};
	printf("[%s][%s]\n",msg,level_msg[level]);
}

static void show_404(int fd)
{
	const char* echo_line = "HTTP/1.0 404 Not Found\r\n";
	send(fd,echo_line,strlen(echo_line),0);
	const char* blank_line = "\r\n";
	send(fd,blank_line,strlen(blank_line),0);
	const char* msg="<html><h1>Not Found!</h1></html>\r\n";
	send(fd,msg,strlen(msg),0);
}
void echo_error(int fd,int errno_num)
{
	switch(errno_num)
	{
		case 200:
			break;
		case 400:
			break;
		case 401:
			break;
		case 403:
			break;
		case 404:
			show_404(fd);
			break;
		case 500:
			break;
		case 503:
			break;
		default:
			break;
	}
} 


int echo_www(int fd,const char* path,int size)
{
	int new_fd=open(path,O_RDONLY);
	if(new_fd < 0)
	{
		print_log("open file error!",FATAL);
		return 404;
	}
	const char* echo_line = "HTTP/1.0 200 OK\r\n";
	send(fd,echo_line,strlen(echo_line),0);
	const char* blank_line = "\r\n";
	send(fd,blank_line,strlen(blank_line),0);
	if(sendfile(fd,new_fd,NULL,size)<0)
	{
		print_log("send file error!",FATAL);
		return 200;
	}
	close(new_fd);
}

int exe_cgi(int fd,const char* method,const char* path,const char* query_string)
{
	int content_len=-1;
	char METHOD[SIZE/10];
	char QUERY_STRING[SIZE];
	char CONTENT_LEN[SIZE];
	if(strcasecmp(method,"GET")==0)
	{
		drop_header(fd);
	}
	else
	{
		char buff[SIZE];
		int ret=-1;
	    do
		{
			ret=get_line(fd,buff,sizeof(buff));
			if(strncasecmp(buff,"Content-Length: ",16)==0)
			{
				content_len = atoi(&buff[16]);
			}
		}while(ret>0 && strcmp(buff,"\n"));
		if(content_len == -1)
		{
			echo_error(fd,404);
			return -1;
		}
	}
	printf("cgi---path: %s\n",path);
	// two pipe be used to delivering info
	int input[2];
	int output[2];
	if(pipe(input)<0)
	{
		echo_error(fd,401);
		return -2;
	}
	if(pipe(output)<0)
	{
		echo_error(fd,401);
		return -3;
	}
	const char* echo_line="HTTP/1.0 200 ok\r\n";
	send(fd,echo_line,strlen(echo_line),0);
	const char* type="Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(fd,type,strlen(type),0);
	const char* blank_line="\r\n";
	send(fd,blank_line,strlen(blank_line),0);

	pid_t id=fork();
	if(id<0)
	{
		echo_error(fd,501);
		return -4;
	}
	else if(id == 0)//child
	{
		close(input[1]);
		close(output[0]);
		sprintf(METHOD,"METHOD=%s",method);
		putenv(METHOD);//export env
		if(strcasecmp(method,"GET")==0)
		{
			sprintf(QUERY_STRING,"QUERY_STRING=%s",query_string);
			putenv(QUERY_STRING);
		}
		else
		{	
			sprintf(CONTENT_LEN,"CONTENT_LEN=%d",content_len);
			putenv(CONTENT_LEN);
		}
		dup2(input[0],0);
		dup2(output[1],1);
		execl(path,path,NULL);
		exit(1);
	}
	else //father
	{
		close(input[0]);
		close(output[1]);

		int i=0;
		char c='\0';
		for(;i<content_len;++i)
		{
			recv(fd,&c,1,0);
			write(input[1],&c,1);
		}
		while(1)
		{
			ssize_t s=read(output[0],&c,1);
			if(s>0)
			{
				send(fd,&c,1,0);
			}
			else
			{
				break;
			}
		}
		waitpid(id,NULL,0);
		close(input[1]);
		close(output[0]);
	}

}

void* handler_request(void* arg)
{
	int fd=(int)arg;
	int errno_num=200;
	int cgi=0;
	char *query_string=NULL;
#ifdef _DEBUG_
	printf("#############################################\n");
	char buff[SIZE];
	int ret=-1;
	do{
		ret=get_line(fd,buff,sizeof(buff));
		printf("%s",buff);
	}while(ret>0 && strcmp(buff,"\n"));
	printf("#############################################\n");
#else
	char method[SIZE/10];
	char url[SIZE];
	char path[SIZE];
	char buff[SIZE];
	int i,j;
	if(get_line(fd,buff,sizeof(buff))<=0)
	{
		print_log("get request line error",FATAL);
		errno_num=501;
		goto end;
	}
	i=0,j=0;
	//get request mathod
	while(i<sizeof(method)-1 && j<sizeof(buff) && !isspace(buff[j]))
	{
		method[i]=buff[j];
		i++;
		j++;
	}
	method[i]=0;
	while(isspace(buff[j]) && j< sizeof(buff))
	{
		j++;
	}
	i=0;
	//get url
	while(i<sizeof(url) && j<sizeof(buff) && !isspace(buff[j]))
	{
		url[i]=buff[j];
		i++;
		j++;
	}
	url[i] = 0;
	printf("method: %s,url: %s\n",method,url);
	if(strcasecmp(method,"GET") && strcasecmp(method,"POST"))
	{
		print_log("method is not ok!",FATAL);
		errno_num=501;
		goto end;
	}
	if(strcasecmp(method,"POST")==0)
	{
		cgi=1;
	}
	query_string=url;
	while(*query_string != 0)
	{
		if(*query_string == '?')
		{
			cgi=1;
			*query_string='\0';
			query_string++;
			break;
		}
		query_string++;
	}
	sprintf(path,"wwwroot%s",url);
	if(path[sizeof(path)-1] == '/')
	{
		strcat(path,"index.html");
	}
	printf("path: %s\n",path);
	//adjust the path is/not exist
	struct stat st;
	if(stat(path,&st)<0)
	{
		print_log("path not found!",FATAL);
		errno_num=404;
		goto end;
	}
	else
	{  // adjust the path is/not a dir
		if(S_ISDIR(st.st_mode))
		{
			strcat(path,"/index.html");
		}
		else
		{ // adjust the path is/not a file.exe
			if((st.st_mode & S_IXUSR) || \
					(st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
			{
				cgi=1;	
			}
		}
    	if(cgi)
    	{
			exe_cgi(fd,method,path,query_string);
		}
		else
		{ // normal--> method=GET && no parameter
			drop_header(fd);
			errno_num=echo_www(fd,path,st.st_size);
		}
	}
end:
	echo_error(fd,errno_num);
	close(fd);
#endif
}











