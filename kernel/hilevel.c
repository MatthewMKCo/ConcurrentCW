#include "hilevel.h"

pcb_t pcb[ 100 ], *current = NULL, *lastLoaded = NULL;
int currentProcess = 0, maxProcesses = 1;
int numberOfPipes = 100;
pipe pipes[100];
/*
current = &pcb[ 0 ];
memcpy( ctx, &pcb[ currentProcess ].ctx, sizeof( ctx_t ) );
lastLoaded = current;
currentProcess = currentProcess + 1;
current = &pcb[ currentProcess ];
*/
void schedule(ctx_t* ctx) {
  while(current -> active == 0){
    currentProcess = (currentProcess + 1) % maxProcesses;
    current = &pcb[ currentProcess ];
  }
  if(current == lastLoaded){
    currentProcess = (currentProcess + 1) % maxProcesses ;
    current = &pcb[ currentProcess];
    return;
  }
  memcpy( &lastLoaded->ctx, ctx, sizeof( ctx_t ) ); // Save
  memcpy( ctx, &pcb[ currentProcess ].ctx, sizeof( ctx_t ) ); // Load
  lastLoaded = &pcb[currentProcess ];
  currentProcess = (currentProcess + 1) % maxProcesses ;
  current = &pcb[ currentProcess ];
  return;
}


void prioritySchedule(ctx_t* ctx) {
  //Increase priority for everything not loaded
  for(int i = 0; i < maxProcesses; i++){
      if(i == currentProcess){
        pcb[ i ].priority = pcb[ i ].originalPriority;
        if(pcb[ i ].active == 0){
          pcb[ i ].priority = 0;
        }
        continue;
      }
      if(pcb[ i ].active == 1){
        pcb[ i ].priority = pcb[ i ].priority + 1;
      }
  }
  memcpy( &pcb[ currentProcess ].ctx, ctx, sizeof( ctx_t ) ); // Save
  //Find next process to load
  for(int i = 0; i < maxProcesses; i++){
    if(pcb[ i ].active == 0)continue;
    else if(pcb[ i ].priority > pcb[ currentProcess ].priority){
      currentProcess = i;
    }
  }
  memcpy( ctx, &pcb[ currentProcess ].ctx, sizeof( ctx_t ) ); // Load

  lastLoaded = &pcb[ currentProcess ];
  return;
}
/*
extern void     main_P3();
extern uint32_t tos_P3;
extern void     main_P4();
extern uint32_t tos_P4;
extern void     main_P5();
extern uint32_t tos_P5;
*/
extern void     main_console();
extern uint32_t tos_console;

void hilevel_handler_rst(ctx_t* ctx) {
  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *
   * - the CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values matche the entry point and top of stack.
   */

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].active   = 1;
  pcb[ 0 ].priority = 5;
  pcb[ 0 ].originalPriority = 5;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_console  );

  for(int i = 0; i < numberOfPipes; i++){
    pipes[i].fd = i + 5;
  }

  memcpy(ctx, &pcb[ currentProcess ].ctx, sizeof(ctx_t));

  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  int_enable_irq();

  return;
}

void hilevel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    prioritySchedule(ctx);
    PL011_putc( UART0, '_', true );    TIMER0->Timer1IntClr = 0x01;
  }

  // Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  /* Based on the identified encoded as an immediate operand in the
   * instruction,
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00 : { // 0x00 => yield()
      prioritySchedule(ctx);
      break;
    }
    case 0x01 : { // 0x01 => write( fd, x, n )

      int   fd = ( int   )( ctx->gpr[ 0 ] );

      int    n = ( int   )( ctx->gpr[ 2 ] );

      if(fd == 1){
        char*  x = ( char* )( ctx->gpr[ 1 ] );
        for( int i = 0; i < n; i++ ) {
          PL011_putc( UART0, *x++, true );
        }
              ctx->gpr[ 0 ] = n;
      }

      else{
        if(pipes[fd - 5].senderFlag == 1 && pipes[fd - 5].receiverFlag == 1){
          void *x = (void*) (ctx->gpr[ 1 ]);
          pipes[fd - 5].write = x;
          pipes[fd - 5].senderFlag = 0;
          pipes[fd - 5].size = n;
          ctx->gpr[ 0 ] = 1;
        }
        else ctx->gpr[ 0 ] = -1;
      }
      break;
    }
    case 0x02 : {// read(fd, x, n)
      int fd = (int)(ctx->gpr[0]);
      int n = (int)(ctx->gpr[2]);
      void* x = (void*)(ctx->gpr[1]);
      if(pipes[fd - 5].senderFlag == 0 && pipes[fd - 5].receiverFlag == 1){
        ctx->gpr[0] = 1;
        pipes[fd - 5].receiverFlag = 0;
        //x = pipes[fd - 5].write;
        memcpy(x, pipes[fd-5].write, pipes[fd - 5].size);
      }
      else ctx->gpr[ 0 ] = -1;
      break;
    }
    case 0x03 : {// fork(x)
      memset(&pcb[maxProcesses], 0, sizeof( pcb_t ));
      memcpy(&pcb[maxProcesses].ctx, ctx, sizeof( ctx_t ) );
      pcb[ maxProcesses ].pid = maxProcesses + 1;
      int x = (int) ctx->gpr[0];
      pcb[ maxProcesses ].priority = x;
      pcb[ maxProcesses ].originalPriority = x;
      pcb[ maxProcesses ].active = 1;
      int spDifference = pcb[maxProcesses].pid - pcb[0].pid;
      pcb[ maxProcesses ].ctx.sp   = ( uint32_t ) pcb[maxProcesses - 1].ctx.sp + (spDifference * 0x00001000);
      pcb[ maxProcesses ].ctx.gpr[0] = 0;
      ctx->gpr[0] = pcb[ maxProcesses ].pid;
      maxProcesses = maxProcesses + 1;
      break;
    }
    case 0x04 : {// exit(x)
      //lastLoaded->active = 0;
      pcb[currentProcess].active = 0;
      prioritySchedule(ctx);
      break;
    }
    case 0x05 : {// exec(x)
    //  ctx->sp = &tos_console +  0x00001000;
      uint32_t x = (uint32_t) ctx->gpr[0];
      ctx->pc = x;
      break;
    }
    case 0x06 : {// kill(pid, x)
      break;
    }
    case 0x07 : {// create_Pipe(int sender, int receiver)
      for(int i = 0; i < numberOfPipes; i++){
        if(pipes[i].used == 0){
          pipes[i].pidSender = ( int )ctx->gpr[0];
          pipes[i].pidReceiver = ( int )ctx->gpr[1];
          pipes[i].receiverFlag = 0;
          pipes[i].senderFlag = 0;
          pipes[i].used = 1;
          ctx->gpr[0] = pipes[i].fd;
          break;
        }
      }
      break;
    }
    case 0x08 : {// open_Pipe(int fd)
      int fd = ctx->gpr[0];
      for(int i = 0; i < numberOfPipes; i++){
        if(fd == 3){
          if(pipes[i].pidSender == pcb[currentProcess].pid){
            pipes[i].senderFlag = 1;
            ctx->gpr[0] = pipes[i].fd;
            break;
          }
          else if(i == numberOfPipes - 1)ctx->gpr[0] = -1;
        }
        else if(fd == 4){
          if(pipes[i].pidReceiver == pcb[currentProcess].pid){
            pipes[i].receiverFlag = 1;
            ctx->gpr[0] = pipes[i].fd;
            break;
          }
          else if(i == numberOfPipes - 1)ctx->gpr[0] = -1;
        }
      }
      break;
    }
    case 0x09 : {// close(int fd)
      int fd = ctx->gpr[0];
      if(pipes[fd - 5].receiverFlag == 0 && pipes[fd - 5].senderFlag == 0){
        pipes[fd - 5].used = 0;
        ctx->gpr[0] = 1;
      }
      else ctx->gpr[0] = -1;
      break;
    }
    case 0x15 : {// get_PID()
      ctx->gpr[0] = pcb[currentProcess].pid;
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
