#include "helper2.h"
#include <string.h>
#include <stdio.h>

int pomnozi_normalno(int a, int b)
{
	printf("Pozvana funkcija pomnozi_normalno\n");
	return a*b;
}

int pomnozi_sabiranjem(int a, int b)
{
	printf("Pozvana funkcija pomnozi_sabiranjem\n");
	int i;
	int result;
	for (i=0; i<a; i++)
	{
		result += b;
	}
	return result;
}
