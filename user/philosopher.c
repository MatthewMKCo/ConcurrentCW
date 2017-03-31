#include "philosopher.h"

void main_Phil(){

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
  //yield();
  int forkreq[1] = {1};
  create_Pipe(thisPhil, leftPhil);
  create_Pipe(thisPhil, rightPhil);
  write(STDOUT_FILENO, "Philosopher ", 12);
  char* ID2;
  itoa(ID2, thisPhil);
  write(STDOUT_FILENO, ID2, sizeof(ID2));
  write(STDOUT_FILENO, " is ready.\n", 11);
  yield();
  while(1){
    ////////////////////////////////////////////////////////////////////////////////////////////////
    int open = 0, writeCheck = -1, readCheck = -1, thinkingCheck = 0;
    int* buf2;
    int* buf3;
    int fd2 = open_Pipe(WRONLY, thisPhil, leftPhil);; //WRITE current philosoper to left philosopher
    int fd3 = open_Pipe(RDONLY, rightPhil, thisPhil);; //READ right philosopher to current philosopher
    int fd4 = open_Pipe(WRONLY, thisPhil, rightPhil);; //WRITE current philosopher to right philosopher
    int fd5 = open_Pipe(RDONLY, leftPhil, thisPhil);; //READ left philosopher to current philosopher
    int readCheckfromfd3 = -1, readCheckfromfd5 = -1;
    int writeChecktofd2 = -1, writeChecktofd4 = -1;
    int forksent[1] = {2};
    int reset = 0;
    //write(STDOUT_FILENO, "work", 4);
    //1 = fork request
    //2 = fork sent
    ////////////////////////////////////////////////////////////////////////////////////////////////
      yield();
      while(rightFork == 1 && leftFork == 0){
        //Receives message from the philosopher on his left
        if(readCheckfromfd5 == -1){
          readCheckfromfd5 = read(fd5, buf3, 1);
        }
        //Fork sent by philosopher on the left
        if(readCheckfromfd5 != -1 && buf3[0] == 2){
          leftFork = 1;
          leftDirty = 1;
          reset = 1;
          continue;
        }
        //Receives message from the philosopher on his right
        if(readCheckfromfd3 == -1){
          readCheckfromfd3 = read(fd3, buf2, 1);
        }
        //Fork request given by philosopher on his right, blocking sending
        //request to the philosopher on his left and sending fork over after
        //cleaning to the right
        if(readCheckfromfd3 != -1 && buf2[0] == 1){
          //block_Pipe(WRONLY, thisPhil, leftPhil);
          //PL011_putc(UART0, thisPhil + '0', true);
          write_Pipe(fd4 ,forksent, sizeof(forksent));
          rightFork = 0;
          reset = 1;
          continue;
        }
        //send a fork request to the philosopher on his left
        //if(writeChecktofd2 == -1){
        write_Pipe(fd2, forkreq, sizeof(forkreq));
          //block_Pipe(RDONLY, rightPhil, thisPhil);

        yield();
      }

///////////////////////////////////////////////

    if(reset == 1){
      yield();
      continue;
}

////////////////////////////////////////////////
//wants right fork
    while(rightFork == 0 && leftFork == 0){
      char* ID;
      itoa(ID, thisPhil);
      while(thinkingCheck == 0){
        write(STDOUT_FILENO, ID, sizeof(ID));
        write(STDOUT_FILENO, " is thinking", 12);
        write(STDOUT_FILENO, "\n", 1);
        thinkingCheck = 1;
      }

      if(readCheckfromfd5 == -1){
        readCheckfromfd5 = read(fd5, buf3, 1);
      }
      //Fork sent by philosopher on the left
      if(readCheckfromfd5 != -1 && buf3[0] == 2){
        leftFork = 1;
        leftDirty = 1;
        reset = 1;
        continue;
      }
      //Receives message from the philosopher on his right
      if(readCheckfromfd3 == -1){
        readCheckfromfd3 = read(fd3, buf2, 1);
      }
      //Fork request given by philosopher on his right, blocking sending
      //request to the philosopher on his left and sending fork over after
      //cleaning to the right
      if(readCheckfromfd3 != -1 && buf2[0] == 1){
        write_Pipe(fd4 ,forksent, sizeof(forksent));
        rightFork = 0;
        reset = 1;
        continue;
      }
      //send a fork request to the philosopher on his left
      //if(writeChecktofd2 == -1){
      write_Pipe(fd2, forkreq, sizeof(forkreq));

      yield();
    }

////////////////////////////////////////////////


  if(reset == 1){
    yield();
    continue;
  }


////////////////////////////////////////////////


    while(rightFork == 1 && leftFork == 1){
      char* ID;
      itoa(ID, thisPhil);
      while(leftDirty == 1 && rightDirty == 1){

        write(STDOUT_FILENO, ID, sizeof(ID));
        write(STDOUT_FILENO, " is eating", 10);
        write(STDOUT_FILENO, "\n", 1);
        leftDirty = 0;
        rightDirty = 0;
      }
      while(rightFork == 1 || leftFork == 1){

        while(rightFork == 1){
          //fork requests read in
          if(readCheckfromfd3 == -1){
            readCheckfromfd3 = read(fd3, buf2, 1);
          }
          //forks sent to other philosophers
          if(readCheckfromfd3 != -1 && buf2[0] == 1){
            write_Pipe(fd4, forksent, sizeof(forksent));
            rightFork = 0;
          }
        }

        while(leftFork == 1){
          //fork requests read in
          if(readCheckfromfd5 == -1){
            readCheckfromfd5 = read(fd5, buf2, 1);
          }
          //forks sent to other philosophers
          if(readCheckfromfd5 != -1 && buf2[0] == 1){
            write_Pipe(fd2, forksent, sizeof(int));
            leftFork = 0;
            reset = 1;
          }
        }

      }

    }

    if(reset == 1){
      yield();
      continue;
    }

    while(leftFork == 1 && rightFork == 0){

      //Receives message from the philosopher on his right
      if(readCheckfromfd3 == -1){
        readCheckfromfd3 = read(fd3, buf3, 1);
      }
      //Fork sent by philosopher on the left
      if(readCheckfromfd3 != -1 && buf3[0] == 2){
        rightFork = 1;
        rightDirty = 1;
        reset = 1;
        continue;
      }
      //send a fork request to the philosopher on his right
      write_Pipe(fd4, forkreq, sizeof(forkreq));
      yield();
    }


      if(reset == 1){
        yield();
        continue;
      }


  }

  exit(EXIT_SUCCESS);
}
