#include "philosopher.h"

void main_phil(){
  create_Pipe();
  int fork = 0;
  int eating = 0;
  int openRightPipe = 0;
  int openLeftPipe = 0;
  int fd[2];
  while(1){
    if(openRightPipe == 0){
      fd[0] = open();
      open = 1;
    }
    if(open == 1){
      write(fd[1], "Give Fork", 9);
      close();
      open = 0;
    }

  }
}
