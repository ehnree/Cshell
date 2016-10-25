#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

int main()
{
  char* p;
  char* pl = "echo ay;echo lmao;\necho oh shit waddup";
  p = strtok(pl, ";");
  while (p)
  {
    printf("%s\n", p);
    p =  strtok(NULL, ";");
  }
  return 0;
}
