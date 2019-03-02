#include <stdio.h>
#inculde <unistd.h>

int main(void) {
  char buff[];

  printf("%s",getcwd(buff, 1024));
  return 0;
}