#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	FILE* fp;
	float percentage;
	long int period = 20000L;
	sscanf(argv[1],"%f",&percentage);
	while(1) 
	{
		fp = fopen("/proc/myled", "w");
		if(fp == NULL)
		{
			printf("Cannot open /proc/myled for write\n");
			return -1;
		}
		fputs("0x0F\n", fp);
		fclose(fp);
		usleep(percentage*period);

		fp = fopen("/proc/myled", "w");
		if(fp == NULL)
		{
			printf("Cannot open /proc/myled for write\n");
			return -1;
		}
		fputs("0x00\n", fp);
		fclose(fp);
		usleep((1-percentage)*period);
	}
	return 0;
}

