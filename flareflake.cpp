#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{  
  FILE *flarefilefilename; // The file containing the name of the file. Not very elegant, but working for now
  char flarefilename[20];  // The name of the file containing the flare

  printf("Running FlareGenerator.py\n\n");
  system("python3 ./FlareGenerator.py"); //Run FG to make txt file
  
  flarefilefilename=fopen("./filename.txt","r"); //Opening file containing flare file name
  if (!flarefilename){
    printf("File containing name of flare file not found\n");
    return 1;
  }
  
  int pos=0;
  while(feof(flarefilefilename)!=1){
    flarefilename[pos]=(fgetc(flarefilefilename));
   pos++;
  }
  flarefilename[pos-1]='\0'; //String is too long, null terminating
  fclose(flarefilefilename);
  
  char command[strlen(flarefilename)+strlen("rm filename.txt; cd Flake-master/; flake -d ../")+2];

  //Creating string to pass to system() as it will only take one argument
  strcpy(command, "rm filename.txt; cd Flake-master/; ./flake -d ../\'");
  strcat(command, flarefilename);
  strcat(command, "\'");
  printf("\nRunning Flake\n\n");
  system(command);
  return 0;
}
