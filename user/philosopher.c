#include "philosopher.h"

extern void main_P3();

void main_Philosopher(){
//  for(int i=0; i<1; i++){
    int pid = fork(5);

    if( 0 == pid ){
      exec(&main_P3);
    }
//  }
  exit( EXIT_SUCCESS );
}
