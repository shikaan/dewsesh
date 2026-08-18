#include "spawn.h"
#include "log.h"
#include "result.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void exec(const char *command) {
  char *cmd = strdup(command);
  char *args[64];
  size_t nargs = 0;

  char *arg;
  char *arg0 = strtok(cmd, " ");
  args[nargs++] = arg0;

  while (nargs < sizeof(args) - 1 && (arg = strtok(NULL, " "))) {
    args[nargs] = arg;
    nargs++;
  }
  args[nargs] = NULL;

  execvp(arg0, args);
}

result_t spw_launch(const char *command) {
  log_debug("launching: %s", command);
  int fds[2];

  if (pipe(fds) < 0) {
    log_error("cannot create pipe: %s", strerror(errno));
    return ERR_SPW_PIPE;
  }

  if (fcntl(fds[1], F_SETFD, FD_CLOEXEC) < 0) {
    log_error("cannot set pipe close-on-exec: %s", strerror(errno));
    return ERR_SPW_PIPE;
  }

  pid_t pid = fork();

  if (pid == 0) {
    close(fds[0]);
    exec(command);
    int err = errno;
    write(fds[1], &err, sizeof(err));
    exit(127);
  }

  close(fds[1]);
  int err = 0;
  ssize_t n = read(fds[0], &err, sizeof(err));
  close(fds[0]);

  if (n > 0) {
    log_error("cannot run selected command: %s", strerror(err));
    return ERR_SPW_EXEC;
  }

  return OK;
}
