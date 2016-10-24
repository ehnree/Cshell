#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    sleep(5);
    printf("I am a child process!\n");
    printf("Args passed:\n");

    for (int i = 0; i < argc; i++)
      printf("%s\n", argv[i]);

    return 0;
}
