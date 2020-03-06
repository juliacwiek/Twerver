#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  /* The user will type in a user name on one line followed by a password 
     on the next.
     DO NOT add any prompts.  The only output of this program will be one 
	 of the messages defined above.
   */

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  
  // initialize fork and pipe params
  int r, fd[2];

  // pipe before fork 
  if (pipe(fd) == -1) {
      perror("pipe");
      exit(1);
  }
  if ((r = fork()) == -1) {
    perror("fork");
    exit(1);
  }

  // if parent process
  if (r > 0) {

    // close output end
    if (close(fd[0]) == -1) {
        perror("close");
        exit(1);
    }
    
    // write to input end
    if (write(fd[1], user_id, MAX_PASSWORD) == -1) {
        perror("write to pipe input");
        exit(1);
    }
    
    if (write(fd[1], password, MAX_PASSWORD) == -1) {
        perror("write to pipe input");
        exit(1);
    }

    int status;
    if (wait(&status) == -1) {
      perror("wait");
      exit(1);
    } else {
      if (WIFEXITED(status)){
        if (WEXITSTATUS(status) == 0){
            printf(SUCCESS);
        } else if (WEXITSTATUS(status) == 2) {
            printf(INVALID);
        } else if (WEXITSTATUS(status) == 3) {
            printf(NO_USER);
        }
      } else {
        perror("child exit");
        exit(1);
      }
    }
    
  } else {
    if (close(fd[1]) == -1) {
        perror("close");
        exit(1);
    }

    // stdin will be a new  copy of fd[0]
    dup2(fd[0], fileno(stdin));

    // we can now close fd[0]
    if (close(fd[0]) == -1) {
        perror("close");
        exit(1);
    }
    execl("./validate", "validate", NULL);

  }

  return 0;
}