#include "middleman.h"

extern void main_Phil();

void main_Middleman(){
  int originalPID = get_PID();
  int numberOfProcesses = 2;
  int childID[numberOfProcesses];

  for(int i = 0; i < numberOfProcesses; i++){
    int pid = fork(10);

    if(0 == pid){
      exec(&main_Phil);
      write(STDOUT_FILENO, "wrong", 5);
    }
    else{
      for(int i = 0; i < numberOfProcesses; i++){
        if(childID[i] == NULL){
          childID[i] = pid;
          break;
        }
      }
    }
  }

  create_Pipe(originalPID, childID[0]);
  int fd = open_Pipe(WRONLY);
  while(1){
    int check = write(fd, 1, 1);
    if(check == 1)break;
  }

  create_Pipe(originalPID, childID[numberOfProcesses - 1]);
  fd = open_Pipe(WRONLY);
  while(1){
    int check = write(fd, 1, 1);
    if(check == 1)break;
  }

  write(STDOUT_FILENO,"middle", 6);
  exit(EXIT_SUCCESS);
}
