#include "middleman.h"

extern void main_Phil();

void main_Middleman(){
  int originalPID = get_PID();
  int numberOfProcesses = 2;
  int childID[numberOfProcesses];

  for(int i = 0; i < numberOfProcesses; i++){
    int pid = fork(10);
    int fd;
    int size2 = sizeof(pid);

    if(0 == pid){
      while(1){
        int pid2 = get_PID();
        fd = open_Pipe(RDONLY, originalPID, get_PID());
        if(fd != -1)break;
        write(STDOUT_FILENO, "wrong", 5);
        PL011_putc(UART0, pid2 + '0', true);
        write(STDOUT_FILENO, "hi", 2);
        yield();
      }
      exec(&main_Phil);

    }
    else{
      for(int i = 0; i < numberOfProcesses; i++){
        if(childID[i] == 0){
          childID[i] = pid;
          break;
        }
      }
    }
  }

  for(int i = 0; i < numberOfProcesses; i++){
    create_Pipe(originalPID, childID[i]);
  }
  for(int i = 0; i < numberOfProcesses; i++){

    int fd = open_Pipe(WRONLY, originalPID, childID[i]);
    int send[2];
    int size = sizeof(send);
    if(i == 0){
      send[0] = childID[numberOfProcesses - 1];
      send[1] = childID[i + 1];
    }
    else if(i == numberOfProcesses - 1){
      send[0] = childID[i - 1];
      send[1] = childID[0];
    }
    else{
      send[0] = childID[i - 1];
      send[1] = childID[i + 1];
    }
    while(1){
      int check = write(fd, send, size);
      if(check == 1)break;
      yield();
    }
    while(1){
      int check = close(fd);
      if(check == 1)break;
      yield();
    }
  }


/*
  create_Pipe(originalPID, childID[0]);
  int fd = open_Pipe(WRONLY);
  while(1){
    int check = write(fd, 1, 1);
    if(check == 1)break;
    yield();
  }
  while(1){
    int check = close(fd);
    if(check == 1)break;
    yield();
  }

  create_Pipe(originalPID, childID[numberOfProcesses - 1]);
  fd = open_Pipe(WRONLY);
  while(1){
    int check = write(fd, 1, 1);

    if(check == 1)break;
    yield();
  }
  while(1){
    int check = close(fd);
    if(check == 1)break;
    yield();
  }
*/
  write(STDOUT_FILENO,"middle", 6);
  exit(EXIT_SUCCESS);
}
