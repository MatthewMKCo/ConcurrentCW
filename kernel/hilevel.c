#include "hilevel.h"

pcb_t pcb[ 100 ], *current = NULL, *lastLoaded = NULL;
int currentProcess = 1, maxProcesses = 4;
/*
current = &pcb[ 0 ];
memcpy( ctx, &pcb[ currentProcess ].ctx, sizeof( ctx_t ) );
lastLoaded = current;
currentProcess = currentProcess + 1;
current = &pcb[ currentProcess ];
*/
void schedule(ctx_t* ctx) {
  if(current -> active == 1 && currentProcess != maxProcesses - 1){
    memcpy( &lastLoaded->ctx, ctx, sizeof( ctx_t ) ); // Save
    memcpy( ctx, &pcb[ currentProcess ].ctx, sizeof( ctx_t ) ); // Load
    lastLoaded = &pcb[ currentProcess ];
    currentProcess = currentProcess + 1;
    current = &pcb[ currentProcess ];
  }
  else if(current -> active == 0 && currentProcess != maxProcesses - 1){
    currentProcess = currentProcess + 1;
    current = &pcb[ currentProcess ];
  }
  else if(current -> active == 0 && currentProcess == maxProcesses - 1){
    currentProcess = 1;
    current = &pcb[ currentProcess ];
  }
  else if(current -> active == 1 && currentProcess == maxProcesses - 1){
    memcpy( &lastLoaded->ctx, ctx, sizeof( ctx_t ) ); // Save
    memcpy( ctx, &pcb[ currentProcess ].ctx, sizeof( ctx_t ) ); // Load
    lastLoaded = &pcb[ currentProcess ];
    currentProcess = 1;
    current = &pcb[ currentProcess ];
  }
  return;
}

extern void     main_P3();
extern uint32_t tos_P3;
extern void     main_P4();
extern uint32_t tos_P4;
extern void     main_P5();
extern uint32_t tos_P5;
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
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_console  );

  memset( &pcb[1], 0, sizeof( pcb_t ));
  pcb[ 1 ].pid      = 2;
  pcb[ 1 ].active   = 1;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( &main_P3 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )( &tos_P3  );

  memset( &pcb[ 2 ], 0, sizeof( pcb_t ) );
  pcb[ 2 ].pid      = 3;
  pcb[ 2 ].active   = 1;
  pcb[ 2 ].ctx.cpsr = 0x50;
  pcb[ 2 ].ctx.pc   = ( uint32_t )( &main_P4 );
  pcb[ 2 ].ctx.sp   = ( uint32_t )( &tos_P4  );

  memset( &pcb[ 3 ], 0, sizeof( pcb_t ) );
  pcb[ 3 ].pid      = 4;
  pcb[ 3 ].active   = 1;
  pcb[ 3 ].ctx.cpsr = 0x50;
  pcb[ 3 ].ctx.pc   = ( uint32_t )( &main_P5 );
  pcb[ 3 ].ctx.sp   = ( uint32_t )( &tos_P5  );

  memcpy(ctx, &pcb[ 1 ].ctx, sizeof(ctx_t));
  lastLoaded = &pcb[1];
  currentProcess = currentProcess + 1;
  current = &pcb[ 2 ];

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
    schedule(ctx);
    PL011_putc( UART0, ' ', true );    TIMER0->Timer1IntClr = 0x01;
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
      break;
    }
    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );
      char*  x = ( char* )( ctx->gpr[ 1 ] );
      int    n = ( int   )( ctx->gpr[ 2 ] );

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }

      ctx->gpr[ 0 ] = n;
      break;
    }/*
    case 0x02 : {// read(fd, x, n)

    }
    case 0x03 : {// fork()

    }
    case 0x04 : {// exit(x)

    }
    case 0x05 : {// exec(x)

    }
    case 0x06 : {// kill(pid, x)

    }*/
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
