#include "philosopher.h"

void main_Phil(){
  /*
  while(1){
    int PID = create_Pipe(1);
    if(PID == -1){
      //PL011_putc(UART0, '0', true);
      continue;
    }
    else break;
  }
  */

  int rightPhil, leftPhil;
  int rightFork = 1, leftFork = 0;
  int rightDirty = 1, leftDirty = 1;
  int thisPhil = get_PID();

  int fd;
  while(1){
    fd = open_Pipe(RDONLY);
    if(fd != -1)break;
    yield();
  }
  write(STDOUT_FILENO, "hi", 2);

  int hi[2];
  int buf;
  while(1){
    buf = read(fd, hi, sizeof(hi));
    if(buf != -1)break;
    yield();
  }
  PL011_putc(UART0, hi[1] + '0', true);

  leftPhil = hi[0];
  rightPhil = hi[1];
  while(1){
    if(rightFork == 0){
      create_Pipe(thisPhil, rightPhil);
      while(1){
        fd = open_Pipe(WRONLY);
        if(fd != -1)break;
        yield();
      }
    }

    if(leftFork == 0){
      create_Pipe(thisPhil, rightPhil);
      while(1){
        fd = open_Pipe(WRONLY);
        if(fd != -1)break;
        yield();
      }
    }
    create_Pipe(thisPhil,leftPhil);
    open_Pipe(RDONLY);
  }
  //open_Pipe(PID);
  exit(EXIT_SUCCESS);
}
