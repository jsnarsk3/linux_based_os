#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#ifndef C_ASM
// Division Error Assembly Linkage
extern void DIVIDE_ERROR();

// NMI Error Assembly Linkage
extern void NMI_INTERRUPT();

// Breakpoint Assembly Linkage
extern void BREAKPOINT();

// Overflow Error Assembly Linkage
extern void OVERFLOW();

// Bound Range Error Assembly Linkage
extern void BOUND_RANGE();

// Invalid Opcode Error Assembly Linkage
extern void INVALID_OPCODE();

// Device Not Available Error Assembly Linkage
extern void DEVICE_NOT_AVAILABLE();

// Double Fault Error Assembly Linkage
extern void DOUBLE_FAULT();

// Invalid TSS Error Assembly Linkage
extern void INVALID_TSS();

// Segment Missing Error Assembly Linkage
extern void SEGMENT_MISSING();

// Stack Segmentation Fault Error Assembly Linkage
extern void STACK_SEG_FAULT();

// General Protection Error Assembly Linkage
extern void GENERAL_PROT();

// Page Fault Error Assembly Linkage
extern void PAGE_FAULT();

// Floating Point Error Assembly Linkage
extern void FLOAT_ERROR();

// Alignment Check Error Assembly Linkage
extern void ALIGN_CHECK();

// Machine Check Error Assembly Linkage
extern void MACHINE_CHECK();

// Floating Point Exception Assembly Linkage
extern void FLOAT_EXCEP();

// System Call Handler Assembly Linkage
extern int SYS_CALL_HANDLER(int call_number, int arg1, int arg2, int arg3);

// RTC Handler Assembly Linkage
extern void RTC_HANDLER();

// Keyboard Interrupt handler Assembly Linkage
extern void KEY_HANDLER();

#endif

#endif
