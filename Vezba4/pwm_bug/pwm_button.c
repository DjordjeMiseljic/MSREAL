#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	FILE* led_p;
	FILE* button1_p;
	FILE* button2_p;
	float percentage=0;
	long int period = 2000L;

	sscanf(argv[1],"%f",&percentage);
	char *mychar1;
	char *mychar2;
	int bytes_read;
	int nbytes=1;
	int b1sd=1;
	int b2sd=1;
	mychar1 = (char*)malloc(nbytes+1);
	mychar2 = (char*)malloc(nbytes+1);
	while(1) 
	{
			
		button1_p = fopen("/sys/class/gpio/gpio956/value", "r");
		bytes_read = getline(&mychar1,&nbytes, button1_p);
		fclose(button1_p);
		int b1v = atoi(mychar1);
		if(b1sd==1 && b1v==1)
		{
			b1sd=0;
			if(b1v && percentage<=1)
				percentage+=0.2;
			if(percentage>1)
				percentage=1;

		}
		if(b1sd==0 && b1v==0)
			b1sd=1;

		button2_p = fopen("/sys/class/gpio/gpio957/valeu", "r");
		bytes_read = getline(&mychar2,&nbytes,button2_p);
		fclose(button2_p);
		int b2v = atoi(mychar2);

		if(b2sd==1 && b2v==1)
		{
			b2sd=0;
			if(b2v && percentage>=0)
				percentage-=0.2;
			if(percentage<0)
				percentage=0;
		}
		if(b2sd==0 && b2v==0)
			b2sd=1;

		led_p = fopen("/proc/myled", "w");
		if(led_p == NULL)
		{
			printf("Cannot open /proc/myled for write\n");
			return -1;
		}
		fputs("0x0F\n", led_p);
		fclose(led_p);
		usleep(percentage*period);

		led_p = fopen("/proc/myled", "w");
		if(led_p == NULL)
		{
			printf("Cannot open /proc/myled for write\n");
			return -1;
		}
		fputs("0x00\n", led_p);
		fclose(led_p);
		usleep((1-percentage)*period);
	}
	return 0;
}

