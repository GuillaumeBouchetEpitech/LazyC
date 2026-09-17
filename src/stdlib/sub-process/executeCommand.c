
#include "./executeCommand.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // waitpid()/WIFEXITED/WEXITSTATUS/WIFSIGNALED/WTERMSIG

extern char **environ;

int executeCommand(const char *inCommand, char *const*inArgv, const char *inCwd)
{
  // char *argv[] = {"ls", "-lah", NULL};
  // char *envp[] = {"PATH=/bin:/usr/bin", NULL};

  pid_t pid = fork();
  if (pid < 0)
  {
    perror("fork");
    return -1;
  }

  if (pid == 0)
  {
    // child process logic
    if (chdir(inCwd) < 0)
    {
      perror("chdir");
      _exit(126);
    }
    // execve("/bin/ls", argv, envp);
    execve(inCommand, inArgv, environ);
    // unreachable unless error
    perror("execve");
    _exit(127);
  }

  // parent process logic
  int status;
  if (waitpid(pid, &status, 0) < 0)
  {
    perror("waitpid");
    return -1;
  }

  if (WIFEXITED(status))
  {
    printf("exited with %d\n", WEXITSTATUS(status));
    return status;
  }
  else if (WIFSIGNALED(status))
  {
    printf("killed by signal %d\n", WTERMSIG(status));
    return -1;
  }

  return 0;
}