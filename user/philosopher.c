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
  PL011_putc(UART0, '0', true);

  //open_Pipe(PID);
  exit(EXIT_SUCCESS);
}
