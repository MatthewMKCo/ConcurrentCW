/* Each of the following is a low-level interrupt handler: each one is
 * tasked with handling a different interrupt type, and acts as a sort
 * of wrapper around a high-level, C-based handler.
 */

.global lolevel_handler_rst
.global lolevel_handler_svc
.global lolevel_handler_irq

lolevel_handler_rst: bl    int_init                @ initialize interrupt vector table,

                     msr   cpsr, #0xD2             @ enter IRQ mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_irq            @ initialize IRQ mode stack
                     msr   cpsr, #0xD3             @ enter SVC mode with IRQ and FIQ interrupts disabled
                     ldr   sp, =tos_svc            @ initialize SVC mode stack

                     sub   sp, sp, #68             @ Initialize dummy context

                     mov   r0, sp                  @ set    high-level C function arg. = SP
                     bl    hilevel_handler_rst     @ invoke high-level C function

                     ldmia sp!, { r0, lr }         @ load   USR mode PC and CPSR
                     msr   spsr, r0                @ set    USR mode        CPSR
                     ldmia sp, { r0-r12, sp, lr }^ @ load   USR mode registers
                     add   sp, sp, #60             @ update SVC mode SP
                     movs  pc, lr                  @ return from interrupt


@ pc stored in SVC lr, lr stored on SVC Stack, lr copies to execution context
@ lr of new Process stored on SVC Stack in High Level Handler, lr pulled into SVC register, lr pushed into pc
lolevel_handler_svc: sub   lr, lr, #0              @ PC is saved to LR on interrupt Call (unknown to us), Corrects LR Value
                     sub   sp, sp, #60             @ Allocate SVC Stack Space for Execution Context of USR process
                     stmia sp, { r0-r12, sp, lr }^ @ Store  USR registers on SVC Stack (Context)
                     mrs   r0, spsr                @ Get USR CPSR
                     stmdb sp!, { r0, lr }         @ Store  USR PC and CPSR on SVC Stack (Context)

                     mov   r0, sp                  @ Set    high-level C function arg. = SP
                     ldr   r1, [ lr, #-4 ]         @ Load   svc instruction
                     bic   r1, r1, #0xFF000000     @ Set    high-level C function arg. = svc immediate
                     bl    hilevel_handler_svc     @ Invoke high-level C function

                     ldmia sp!, { r0, lr }         @ Load   PC & CPSR from SVC Stack, Save to r0 & Lr of SVC Register File
                     msr   spsr, r0                @ Set    CPSR to SPSR (Cannot do this directly because SPSR is special purpose)
                     ldmia sp, { r0-r12, sp, lr }^ @ Load SVC Stack (Context) onto USR Mode Register File
                     add   sp, sp, #60             @ Update SVC mode SP
                     movs  pc, lr                  @ Return from interrupt, SVC lr to PC


lolevel_handler_irq: sub   lr, lr, #4
                     sub   sp, sp, #60
                     stmia sp, { r0-r12, sp, lr }^
                     mrs   r0, spsr
                     stmdb sp!, { r0, lr }

                     mov   r0, sp                  @ Set    high-level C function arg0 = SP
                     bl    hilevel_handler_irq     @ Invoke high-level C function

                     ldmia sp!, { r0, lr }
                     msr   spsr, r0
                     ldmia sp, { r0-r12, sp, lr }^
                     add   sp, sp, #60
                     movs  pc, lr
