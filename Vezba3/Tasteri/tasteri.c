#include <stdio.h>
#include <stdlib.h>

int main ()
{
  int rval;
  FILE *fp1;
  FILE *fp2;
  char *str;
  int tval1,tval2;
  size_t num_of_bytes = 1;

  while(1)
  {

	 //Citanje vrednosti prvog tastera
	 fp1 = fopen ("/sys/class/gpio/gpio956/value", "r");
	 if(fp1==NULL)
		 puts("Problem pri otvaranju 1. fajla");

    str = (char *)malloc(num_of_bytes+1); 
    getline(&str, &num_of_bytes, fp1); 

	 if(fclose(fp1))
		 puts("Problem pri zatvaranju 1. fajla");

    sscanf(str, "%d", &tval1); 
    free(str);

	 //Citanje vrednosti drugog tastera
	 fp2 = fopen ("/sys/class/gpio/gpio957/value", "r");
	 if(fp2==NULL)
		 puts("Problem pri otvaranju 2. fajla");

    str = (char *)malloc(num_of_bytes+1); 
    getline(&str, &num_of_bytes, fp2); 

	 if(fclose(fp2))
		 puts("Problem pri zatvaranju 2. fajla");

    sscanf(str, "%d", &tval2); 
    free(str);

	 //Ispis procitanih vrednosti
    printf("\n:Taster1: %d		Taster2: %d", tval1, tval2);
	 usleep(100000);

  }
  
  
  
  return 0;
}
