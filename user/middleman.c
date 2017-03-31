#include "middleman.h"

extern void main_Phil();

void main_Middleman(){
  int originalPID = get_PID();
  int numberOfProcesses = 16;
  int childID[16];

  for(int i = 0; i < numberOfProcesses; i++){
    int pid = fork(10);
    int fd;
    int size2 = sizeof(pid);

    if(0 == pid){
      while(1){
        int pid2 = get_PID();
        fd = open_Pipe(RDONLY, originalPID, get_PID());
        if(fd != -1)break;
        /*
        write(STDOUT_FILENO, "wrong", 5);
        PL011_putc(UART0, pid2 + '0', true);
        write(STDOUT_FILENO, "hi", 2);
        */
        yield();
      }
      exec(&main_Phil);

    }
    else{
      for(int y = 0; y < numberOfProcesses; y++){
        if(childID[y] == 0){
          childID[y] = pid;
          create_Pipe(originalPID, childID[y]);
          yield();
          break;
        }
      }
    }
  }

  int send[16][2];
  for(int i = 0; i < numberOfProcesses; i++){
    //create_Pipe(originalPID, childID[i]);
    int fd = open_Pipe(WRONLY, originalPID, childID[i]);

    if(i == 0){
      send[i][0] = childID[numberOfProcesses - 1];
      send[i][1] = childID[i + 1];
    }
    else if(i == numberOfProcesses - 1){
      send[i][0] = childID[i - 1];
      send[i][1] = childID[0];
    }
    else{
      send[i][0] = childID[i - 1];
      send[i][1] = childID[i + 1];
    }
    //send[i][0] = 1 + i;
    //send[i][1] = 5 + i;
    size_t size = sizeof(send[i]);
//    int (*send2)[2] = &send;
//    size_t size2 = sizeof(send2);
//    void* send3 = &send2;
//    size_t size3 = sizeof(send3);
    while(1){
      int check = write_Pipe(fd, send[i], size);
      if(check != -1)break;
      yield();
    }
    //write(STDOUT_FILENO, "through", 7);
    /*
    while(1){
      int check = close(fd);
      if(check == 1)break;
      yield();
    }*/
  }

  exit(EXIT_SUCCESS);
}
