#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char **argv)
{
  int pipefd[2];
  int pipefdSort[2];
  int pid;

  if(argc < 2) {
    fprintf(stderr, "Usage: %s\n", argv[0]);
    return 1;
  }

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL}; 

  // make a pipe (fds go in pipefd[0] and pipefd[1])

  if (pipe(pipefd) == -1 || pipe(pipefdSort) == -1) {
        perror("pipe");
        return 1;
    }

  pid = fork();
  if (pid < 0) {
        perror("fork");
        return 1;
  }

  if (pid == 0)
    {
      pid = fork();

      if (pid < 0) {
        perror("fork");
        return 1;
      }

      if (pid == 0){
        // child's child gets here and handles "sort"
        dup2(pipefdSort[0], 0);
        close(pipefdSort[1]);
         execvp("sort", sort_args); // Execute sort
        return 1;

      } else {
        // child gets here and handles "grep Villanova"
        // replace standard input with input part of pipe
        dup2(pipefd[0], 0);
        dup2(pipefdSort[1], 1);

        // close unused hald of pipe
        close(pipefd[1]);
        close(pipefdSort[0]);

        // execute grep
        execvp("grep", grep_args);

        return 1;
        
      }
    }
  else
    {
      // parent gets here and handles "cat scores"
      // replace standard output with output part of pipe

      dup2(pipefd[1], 1);

      // close unused unput half of pipe

      close(pipefd[0]);
      close(pipefdSort[0]);
      close(pipefdSort[1]);

      // execute cat

      execvp("cat", cat_args);
      return 1;
    }

    wait(NULL);
    wait(NULL);

    return 0;
}