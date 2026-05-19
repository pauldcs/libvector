#include "unit_tests.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#ifdef __linux__
#include <wait.h>
#endif

#define BGRN "\033[1;32m"
#define CRESET "\033[0m"
#define BRED "\033[1;31m"
#define YEL "\033[0;33m"

specs_state_t __specs__ = {
    .passed = 0,
    .failed = 0,
};

bool run_test(bool (*test)(void), const char *title) {
  fprintf(stderr, " test: %s ... ", title);
  int ret;

  pid_t pid = fork();
  if (pid == -1)
    assert(0 && "Internal error");
  else if (pid == 0) {
    alarm(TIMEOUT_LIMIT);
    if (test())
      exit(0);
    exit(1);
  }

  while (waitpid(pid, &ret, 0) != -1)
    continue;

  if (WEXITSTATUS(ret))
    goto test_ko;

  if (WIFSIGNALED(ret)) {
    fprintf(stderr, "SIG %s, ", strsignal(WTERMSIG(ret)));
    goto test_ko;
  }

  __specs__.passed++;
  fprintf(stderr, BGRN "OK %s\n", CRESET);
  return true;

test_ko:
  __specs__.failed++;
  fprintf(stderr, BRED "KO %s\n", CRESET);
  return false;
}
