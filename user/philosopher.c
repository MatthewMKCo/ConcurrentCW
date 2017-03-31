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

  int fd = get_Last_Pipe();

  //write(STDOUT_FILENO, "hi", 2);
  int* buf;
  int size = 2;
  while(1){
    //int check = 0;
    int check = read(fd, buf, 2);
    if(check != -1)break;
//    write(STDOUT_FILENO, "help", 4);
    yield();
  }
//PL011_putc(UART0, thisPhil + '0', true);
//  write(STDOUT_FILENO, "fu", 2);

  //PL011_putc(UART0, buf[1] + '0', true);
  //write(STDOUT_FILENO, "fu", 2);
  leftPhil = buf[0];
  rightPhil = buf[1];
  yield();
  int forkreq[1] = {1};
  create_Pipe(thisPhil, leftPhil);
  create_Pipe(thisPhil, rightPhil);

  while(1){
    ////////////////////////////////////////////////////////////////////////////////////////////////
    int open = 0, writeCheck = -1, readCheck = -1;
    int* buf2;
    int fd2 = -1; //WRITE current philosoper to left philosopher
    int fd3 = -1; //READ right philosopher to current philosopher
    int fd4 = -1; //WRITE current philosopher to right philosopher
    int fd5 = -1; //READ left philosopher to current philosopher
    int readCheckfromfd3 = -1, readCheckfromfd5 = -1;
    int writeChecktofd2 = -1, writeChecktofd4 = -1;
    int forksent[1] = {2};
    int reset = 0;
    //write(STDOUT_FILENO, "work", 4);
    //1 = fork request
    //2 = fork sent
    ////////////////////////////////////////////////////////////////////////////////////////////////
    while(rightFork == 1 && leftFork == 0){
      fd2 = open_Pipe(WRONLY, thisPhil, leftPhil);
      while(rightFork == 1 && leftFork == 0){
        //Open all pipes
        if(fd3 == -1){
          fd3 = open_Pipe(RDONLY, rightPhil, thisPhil);
        }
        if(fd4 == -1){
          fd4 = open_Pipe(WRONLY, thisPhil, rightPhil);
        }
        if(fd5 == -1){
          fd5 = open_Pipe(RDONLY, leftPhil, thisPhil);
        }
        //Receives message from the philosopher on his right
        if(readCheckfromfd3 == -1){
          readCheckfromfd3 = read(fd3, buf2, 1);
        }
        //Receives message from the philosopher on his left
        if(readCheckfromfd5 == -1){
          readCheckfromfd5 = read(fd5, buf2, 1);
        }
        //Fork request given by philosopher on his right, blocking sending
        //request to the philosopher on his left and sending fork over after
        //cleaning to the right
        if(readCheckfromfd3 != -1 && buf2[0] == 1){
          block_Pipe(WRONLY, thisPhil, leftPhil);
          rightDirty = 0;
          write(fd4 ,forksent, sizeof(int));
          rightFork = 0;
          reset = 1;
        }
        //Fork sent by philosopher on the left
        if(readCheckfromfd5 != -1 && buf2[0] == 2){
          leftFork = 1;
          reset = 1;
        }
        //send a fork request to the philosopher on his left
        if(writeChecktofd2 == -1){
          writeChecktofd2 = write(fd2, forkreq, sizeof(int));
        }

      }
    }

    if(reset == 1)continue;

    while(rightFork == 0 && leftFork == 0){

    }

    if(reset == 1)continue;

    while(rightFork == 1 && leftFork == 1){
      PL011_putc(UART0, thisPhil + '0', true);
      //write(STDOUT_FILENO, "work", 4);
    }

    if(reset == 1)continue;
  }

  exit(EXIT_SUCCESS);
}
