#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    printf("I am a child process!\n");
    printf("Args passed:\n");

    for (int i = 0; i < argc; i++)
      printf("%s\n", argv[i]);

    return 0;
}
