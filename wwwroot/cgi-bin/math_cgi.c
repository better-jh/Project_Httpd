#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
	//printf("<html><h1>hello cgi!</h1></html><br/>");
	char *method = NULL;
	char *arg_string = NULL;
	char *content_len = NULL;
	char buff[1024];
	method = getenv("METHOD");
	if(method && strcasecmp(method,"GET")==0)
	{
		arg_string = getenv("QUERY_STRING");
		if(!arg_string)
		{
			printf("get arg_string error!!\n");
			return 1;
		}
		strcpy(buff,arg_string);
	}
	else if(method && strcasecmp(method,"POST")==0)
	{
		content_len = getenv("CONTENT_LEN");
		if(!content_len)
		{
			printf("get content_len error!!\n");
			return 2;
		}
		int i=0;
		char c=0;
		int nums=atoi(content_len);
		for(;i<nums;++i)
		{
			read(0,&c,1);
			buff[i]=c;
		}
		buff[i]=0;
	}
	else
	{
		printf("get method error!!\n");
		return 3;
	}

	//printf("%s\n",buff);
	int nums[2];
	int len=strlen(buff);
	//printf("%d\n",len);
	int idx=1;
	int i=0;
	for(i=len;i>=0;--i)
	{
		if(buff[i-1] == '=')
		{
			nums[idx] = atoi(&buff[i]);
			--idx;
		}
		else if(buff[i-1] == '&')
		{
			buff[i-1]='\0';
		}
	}
	printf("%d + %d = %d<br>", nums[0],nums[1],nums[0]+nums[1]);
	printf("%d - %d = %d<br>", nums[0],nums[1],nums[0]-nums[1]);
	printf("%d * %d = %d<br>", nums[0],nums[1],nums[0]*nums[1]);
	printf("%d / %d = %d<br>", nums[0],nums[1],nums[0]/nums[1]);
	return 0;
}
