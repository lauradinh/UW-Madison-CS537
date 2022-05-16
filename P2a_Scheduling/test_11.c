#include "types.h"
#include "user.h"
#include "pstat.h"

int
get_pstat_idx(struct pstat* pstat, int pid)
{
  for (int i = 0; i < NPROC; i++) {
    if (pstat->pid[i] == pid)
      return i;
  }
  return -1;
}

int
main(int argc, char *argv[])
{
  int pid = getpid();
  struct pstat pstat;

  sleep(10);

  if (getpinfo(&pstat) != 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  int idx = get_pstat_idx(&pstat, pid);
  if (idx < 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  if (pstat.boostsleft[idx] <= 10 && pstat.boostsleft[idx] >= 8)
    printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  else
    printf(1, "XV6_SCHEDULER\t FAILED\n");

  exit();
}
