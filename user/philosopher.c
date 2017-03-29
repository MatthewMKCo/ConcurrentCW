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

  int fd;
  while(1){
    fd = open_Pipe(RDONLY);
    if(fd != -1)break;
    yield();
  }
  write(STDOUT_FILENO, "hi", 2);

  int* hi;
  int buf;
  while(1){
    buf = read(fd, hi, 1);
    if(buf != -1)break;
    yield();
  }
  PL011_putc(UART0, buf + '0', true);


  //open_Pipe(PID);
  exit(EXIT_SUCCESS);
}
