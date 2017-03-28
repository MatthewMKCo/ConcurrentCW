#include "middleman.h"

extern void main_phil();

void main_Middleman(){
  int originalPID = get_PID();
  int numberOfProcesses = 2;

  for(int i = 0; i < numberOfProcesses; i++){
    int pid = fork(10);

    if(0 == pid){
      int currentPID = get_PID();
      create_Pipe(currentPID, originalPID);
    }
  }

  exit(EXIT_SUCCESS);
}
