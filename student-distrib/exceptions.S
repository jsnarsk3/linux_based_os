#define C_ASM 1
#include "exceptions.h"
.text

# Declaring the assembly linkage functions as global
.global DIVIDE_ERROR, NMI_INTERRUPT, BREAKPOINT
.global OVERFLOW, BOUND_RANGE, INVALID_OPCODE, DEVICE_NOT_AVAILABLE
.global DOUBLE_FAULT, INVALID_TSS, SEGMENT_MISSING, STACK_SEG_FAULT
.global GENERAL_PROT, PAGE_FAULT, FLOAT_ERROR, ALIGN_CHECK, MACHINE_CHECK
.global FLOAT_EXCEP

# declaring actual exception handlers as global
.global divide_err, nmi, breakpoint, overflow ,b_range, inv_op
.global dev_not_avail ,db_fault, inv_tss, seg_miss, stack_seg
.global gen_prot, page_fault, float_err, align_chk, machine_chk
.global float_ex, sys_call_handle, keyboard_handler, rtc_handler
.global test_interrupts

.global SYS_CALL_HANDLER, RTC_HANDLER, KEY_HANDLER

jump_table:
.long sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn
.align 4

# Exception 0
# Assembly linkage for exception 0
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 0 and returns with iret
DIVIDE_ERROR:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 2
# Assembly linkage for exception 2
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 2 and returns with iret
NMI_INTERRUPT:
 	movl	$256, %ebx
 	jmp 	sys_halt

# Exception 3
# Assembly linkage for exception 3
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 3 and returns with iret
BREAKPOINT:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 4
# Assembly linkage for exception 4
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 4 and returns with iret
OVERFLOW:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 5
# Assembly linkage for exception 5
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 5 and returns with iret
BOUND_RANGE:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 6
# Assembly linkage for exception 6
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 6 and returns with iret
INVALID_OPCODE:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 7
# Assembly linkage for exception 7
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 7 and returns with iret
DEVICE_NOT_AVAILABLE:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 8
# Assembly linkage for exception 8
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 8 and returns with iret
DOUBLE_FAULT:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 9
# Assembly linkage for exception 9
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 9 and returns with iret
COPROC_SEG:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 10
# Assembly linkage for exception 10
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 10 and returns with iret
INVALID_TSS:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 11
# Assembly linkage for exception 11
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 11 and returns with iret
SEGMENT_MISSING:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 12
# Assembly linkage for exception 12
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 12 and returns with iret
STACK_SEG_FAULT:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 13
# Assembly linkage for exception 13
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 13 and returns with iret
GENERAL_PROT:
	movl	$256, %ebx
	jmp 	sys_halt

# Exception 14
# Assembly linkage for exception 14
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 14 and returns with iret
PAGE_FAULT:
	movl	$256, %ebx
	jmp 	sys_halt


# Exception 16
# Assembly linkage for exception 16
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 16 and returns with iret
FLOAT_ERROR:
	movl	$256, %ebx
	jmp 	sys_halt


# Exception 17
# Assembly linkage for exception 17
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 17 and returns with iret
ALIGN_CHECK:
	movl	$256, %ebx
	jmp 	sys_halt


# Exception 18
# Assembly linkage for exception 18
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 18 and returns with iret
MACHINE_CHECK:
	movl	$256, %ebx
	jmp 	sys_halt


# Exception 19
# Assembly linkage for exception 19
# inputs: none
# outputs: none
# side effects: calls exception handler for exception 19 and returns with iret
FLOAT_EXCEP:
	movl	$256, %ebx
	jmp 	sys_halt


# System Call Handler
# Assembly linkage for system calls
# inputs: none
# outputs: none
# side effects: calls system call handler and returns with iret
SYS_CALL_HANDLER:
	pusha
	pushf
	cmpl $10, %eax      # Check if valid command
	jg INVALID_COMMAND
	cmpl $1, %eax
	jl INVALID_COMMAND
	subl $1, %eax
	jmp *jump_table(, %eax, 4) # Jump to the correct system call
sys_halt:
	push %bx #push args
	call sys_halt_c
	addl $4, %esp
	jmp exec_done
sys_execute:
	pushl %ebx #push args
	call sys_execute_c
exec_done:
	addl $4, %esp
	cmpl $0, %eax
	je  sys_halt
	cmpl $-1, %eax
	je	DONE
	cmpl $256, %eax
	je 	DONE
	jmp EXEC_DONE
sys_read:
	pushl %edx #push args
	pushl %ecx
	pushl %ebx
	call sys_read_c
	addl $12, %esp
	jmp DONE
sys_write:
	pushl %edx #push args
	pushl %ecx
	pushl %ebx
	call sys_write_c
	addl $12, %esp
	jmp DONE
sys_open:
	pushl %ebx #push args
	call sys_open_c
	addl $4, %esp
	jmp DONE
sys_close:
	pushl %ebx #push args
	call sys_close_c
	addl $4, %esp
	jmp DONE
sys_getargs:
	pushl %ecx #push args
	pushl %ebx
	call sys_getargs_c
	addl $8, %esp
	jmp DONE
sys_vidmap:
	pushl %ebx #push args
	call sys_vidmap_c
	addl $4, %esp
	jmp DONE
sys_set_handler:
	pushl %ecx #push args
	pushl %ebx
	call sys_getargs_c
	addl $8, %esp
	jmp DONE
sys_sigreturn:
	call sys_sigreturn_c
	jmp DONE
#Invalid Syscall Number
INVALID_COMMAND:
	movl $-1, %eax

DONE:
#Restore Flags and registers
	movl %eax, ret_val
	popf
	popa
	movl ret_val, %eax
#Maybe need to set iret context?
	iret

EXEC_DONE:
#Restore Flags and registers
	movl %ebx, ret_val
	popf
	popa
	movl ret_val, %eax
#Maybe need to set iret context?
	iret

ret_val:
.long 0x0
# RTC Handler
# Assembly linkage for RTC_HANDLER
# inputs: none
# outputs: none
# side effects: calls RTC_HANDLER and returns with iret
RTC_HANDLER:
	pusha
	pushf
	call rtc_handler
	popf
	popa
	iret

# Keyboard Handler
# Assembly linkage for Keyboard Handler
# inputs: none
# outputs: none
# side effects: calls Keyboard Handler and returns with iret
KEY_HANDLER:
	pusha
	pushf
	call key_handler
	popf
	popa
	iret
