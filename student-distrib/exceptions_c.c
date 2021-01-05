#include "exceptions_c.h"
#include "lib.h"
#include "i8259.h"

/* void divide_err();
 * Inputs: none
 * Return Value: none
 * Function: Prints divide error to screen */
extern void divide_err(){
	clear();
	printf("Divide by Zero Error!\n");
	while(1){};
};
/* void nmi();
 * Inputs: none
 * Return Value: none
 * Function: Prints NMI error to screen */
extern void nmi(){
	clear();
	printf("NMI Interrupt\n");
	while(1){};
};

/* void breakpoint();
 * Inputs: none
 * Return Value: none
 * Function: Prints breakpoint error to screen */
extern void breakpoint(){
	clear();
	printf("BREAKPOINT\n");
	while(1){};
};

/* void overflow();
 * Inputs: none
 * Return Value: none
 * Function: Prints overflow error to screen */
extern void overflow(){
	clear();
	printf("OVERFLOW ERROR\n");
	while(1){};
};

/* void b_range();
 * Inputs: none
 * Return Value: none
 * Function: Prints BOUND_RANGE Error to screen */
extern void b_range(){
	clear();
	printf("BOUND_RANGE ERROR\n");
	while(1){};
};

/* void inv_op();
 * Inputs: none
 * Return Value: none
 * Function: Prints INVALID_OPCODE error to screen */
extern void inv_op(){
	clear();
	printf("INVALID_OPCODE ERROR\n");
	while(1){};
};

/* void dev_not_avail();
 * Inputs: none
 * Return Value: none
 * Function: Prints DEVICE_NOT_AVAILABLE error to screen */
extern void dev_not_avail(){
	clear();
	printf("DEVICE_NOT_AVAILABLE ERROR\n");
	while(1){};
};

/* void db_fault();
 * Inputs: none
 * Return Value: none
 * Function: Prints DOUBLE_FAULT error to screen */
extern void db_fault(){
	clear();
	printf("DOUBLE_FAULT ERROR\n");
	while(1){};
};

/* void inv_tss();
 * Inputs: none
 * Return Value: none
 * Function: Prints INVALID_TSS error to screen */
extern void inv_tss(){
	clear();
	printf("INVALID_TSS ERROR\n");
	while(1){};
};

/* void seg_miss();
 * Inputs: none
 * Return Value: none
 * Function: Prints SEGMENT_MISSING error to screen */
extern void seg_miss(){
	clear();
	printf("SEGMENT_MISSING ERROR\n");
	while(1){};
};

/* void stack_seg();
 * Inputs: none
 * Return Value: none
 * Function: Prints STACK_SEG_FAULT error to screen */
extern void stack_seg(){
	clear();
	printf("STACK_SEG_FAULT ERROR\n");
	while(1){};
};

/* void gen_prot();
 * Inputs: none
 * Return Value: none
 * Function: Prints GENERAL_PROT error to screen */
extern void gen_prot(){
	clear();
	printf("GENERAL_PROT ERROR\n");
	while(1){};
};

/* void page_fault();
 * Inputs: none
 * Return Value: none
 * Function: Prints PAGE_FAULT error to screen */
extern void page_fault(){
	clear();
	printf("PAGE_FAULT ERROR\n");
	// exception_flag = 1;
	while(1){};
};

/* void float_err();
 * Inputs: none
 * Return Value: none
 * Function: Prints FLOATING Point error to screen */
extern void float_err(){
	clear();
	printf("FLOATING POINT ERROR\n");
	while(1){};
};

/* void align_chk();
 * Inputs: none
 * Return Value: none
 * Function: Prints ALIGN_CHECK error to screen */
extern void align_chk(){
	clear();
	printf("ALIGN_CHECK ERROR\n");
	while(1){};
};

/* void machine_chk();
 * Inputs: none
 * Return Value: none
 * Function: Prints MACHINE_CHECK error to screen */
extern void machine_chk(){
	clear();
	printf("MACHINE_CHECK ERROR\n");
	while(1){};
};

/* void float_ex();
 * Inputs: none
 * Return Value: none
 * Function: Prints FLOATING POINT EXCEPTION error to screen */
extern void float_ex(){
	clear();
	printf("FLOATING POINT EXCEPTION ERROR\n");
	while(1){};
};

/* void sys_call_handle();
 * Inputs: 		int call_number: Call number of the system call to service
 *				in arg1, arg2, arg3: Arguments associated with the system call
 *				with the call number.
 * Return Value: Returns 0 is successful and -1 if failure
 * Function: Handles the inputted system call */
extern int sys_call_handle(int call_number, int arg1, int arg2, int arg3){
	// Do the system call
	clear();
	printf("SYSTEM CALL\n");
	while(1){};
	return 0;
}
