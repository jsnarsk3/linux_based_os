#ifndef _EXCEPTIONS_C_H
#define _EXCEPTIONS_C_H

// Division Exception Handler
extern void divide_err();
// NMI exception handler
extern void nmi();
// Breakpoint handler
extern void breakpoint();
// Overflow exception handler
extern void overflow();
// Bound Range exception handler 
extern void b_range();
// Invalid Opcode Exception Handler
extern void inv_op();
// Device Not Available exception handler 
extern void dev_not_avail();
// Double Fault Exception handler 
extern void db_fault();
// Invalid TSS exception handler 
extern void inv_tss();
// Segment Missing Exception Handler 
extern void seg_miss();
// Stack Segmentation Error Handler 
extern void stack_seg();
// General Protection exception handler 
extern void gen_prot();
// Page Fault Exception handler 
extern void page_fault();
// Floating Point Error  handler 
extern void float_err();
// Alignment Check exception handler 
extern void align_chk();
// Machine Check exception handler 
extern void machine_chk();
// Flating Point exception handler 
extern void float_ex();
// System Call handler 
extern int sys_call_handle(int call_number, int arg1, int arg2, int arg3);
// Keyboard interrupt handler 

#endif
