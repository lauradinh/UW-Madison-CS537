// (guanzhou) p4
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int sleepT;

  if (argc != 2) {
    printf(1, "loop: must invoke as `loop sleepT`\n");
    exit();
  }

  sleepT = atoi(argv[1]);

  if (sleepT < 0) {
    printf(1, "loop: sleepT must be positive\n");
    exit();
  }

  // sleep for sleepT ticks
  sleep(sleepT);

  // when waked up, do an infinite loop
  int i = 0, j = 0;
  while (i < 800000000) {
    j += i * j + 1;
    i++;
  }
  int x =i*j;
  printf(1, "loop output (ignore this): %d\n", x);

  exit();
}
