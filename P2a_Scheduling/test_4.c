#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{ 
  srand(537);

  if (settickets(-1, 10) == 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  if (settickets(getpid(), 0) == 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  if (getpinfo((struct pstat*)-1) == 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  exit();
}
