#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main (int argc,char** argv)
{
	printf ("Broj argumenata je %d\nNulti je %s\nPrvi je %s\nDrugi je %s\n",argc,argv[0],argv[1],argv[2]);
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);
	int res = pomnozi(a,b);
	printf("Rezultat mnozenja je: %d\n",res);
}
