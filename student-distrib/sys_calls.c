#include "sys_calls.h"

#define TERMINAL_FILE_FLAGS 0x7 //mark in use, of file type 4, ...000111
#define STDIN_INDEX			0
#define STDOUT_INDEX		1
#define FD_SIZE				7

// System Call 1 - Halt
extern int32_t sys_halt_c(uint8_t status){
	/* assert can close a process and page exists */
	if (process_number <= 0) {
		return -1;
	}
	process_number--;

	pd_entry_bigPage_t* program_page;
	if (NULL == (program_page = get_bigPage(PRO_IDX))) {
		return -1;
	}

	/* clear fd */
	uint32_t fd;
	for (fd = 0; fd < 8; fd++) {
		if ((cur_pcb->file_array)[fd].flags % 2 != 0) {
			((cur_pcb->file_array)[fd].file_op_ptr->close_ptr)(fd);
		}
	}
	/* restore parent data and parent paging */
	tss.esp0 = cur_pcb->old_esp0;
	program_page->page_base_addr = cur_pcb->old_phys_addr;
	lpdt(ret_dir_ptr());
	asm("					\n\
		movl	%0, %%ebp 	\n\
		"
		:
		: "r"(cur_pcb->old_ebp)
	);
 	cur_pcb = (pcb_t*) cur_pcb->old_pcb_ptr;

	return 0xFF & status;
};

// System Call 2 - Execute
extern int32_t sys_execute_c(const uint8_t* command){
	uint8_t file_name[33];
	uint32_t idx;
	dentry_t dentry;
	uint8_t header[32];
	pd_entry_bigPage_t* program_page;

	if (process_number >= 6) {
		return -1;
	}

	/*better fix for newline issue, doesn't read newline into the file_name to be searched for*/
	for (idx = 0; idx < 33; idx++) {
		if (command[idx] == ' ' || command[idx] == 0 || command[idx] == '\n') {
	 		break;
	 	}
		file_name[idx] = command[idx];
	}
	file_name[idx] = 0;

	/* read file and check that it is a regular file */
	if (read_dentry_by_name(file_name, &dentry) == -1) {
		return -1;
	}
	if (dentry.filetype != REG_FILE) {
		return -1;
	}
	/* read header of file, containing ELF (if executable, bytes 0-3) and EIP (bytes 24-27) */
	if (read_data(dentry.inode_num, 0, header, 32, -1) == -1) {
		return -1;
	}
	if (strncmp((int8_t*) elf_text, (int8_t*) header, 4) != 0) {
		return -1;
	}

	/* create new pcb */
	pcb_t new_pcb;
	strcpy((int8_t*) new_pcb.input, (int8_t*) command);

	/** PAGING **/
	/* assert page is present before modifying */
	if (NULL == (program_page = get_bigPage(PRO_IDX))) {
		asm("					\n\
			movl	$-1, %%ebx 	\n\
			"
			:
			:
		);
		return -1;
	}
	/* save old physical address in pcb */
	new_pcb.old_phys_addr = program_page->page_base_addr;
	/* update physical address */
	uint32_t p_addr = ((EIGHT_MB + process_number*FOUR_MB) & EIGHT_MASK) >> EIGHT_S;
	program_page->page_base_addr = p_addr;
	/* flush TLB */
	lpdt(ret_dir_ptr());
	/* copy program to physical memory */
	uint32_t v_addr = USER_PROG;
	uint8_t *v_ptr = (uint8_t*) v_addr;
	read_data(dentry.inode_num, 0, v_ptr, FOUR_MB, -1);

	/** PCB **/
	uint32_t fd_idx;
	/* file array entry for stdin (fd = 0) */
	new_pcb.file_array[0].file_op_ptr = &terminal_table;
	new_pcb.file_array[0].inode = 0;
	new_pcb.file_array[0].file_pos = 0;
	new_pcb.file_array[0].flags = TERMINAL_FILE_FLAGS;
	/* file array entry for stdout (fd = 1) */
	new_pcb.file_array[1].file_op_ptr = &terminal_table;
	new_pcb.file_array[1].inode = 0;
	new_pcb.file_array[1].file_pos = 0;
	new_pcb.file_array[1].flags = TERMINAL_FILE_FLAGS;
	/* open stdin/out */
	terminal_open((uint8_t*) "temp");
	/* clear file array entries [2-8) */
	for (fd_idx = 2; fd_idx < 8; fd_idx++) {
		new_pcb.file_array[fd_idx] = clear_fd;
	}
	/* store cur_pcb ptr */
	new_pcb.old_pcb_ptr = (struct pcb_t*) cur_pcb;
	new_pcb.old_esp0 = tss.esp0;


	/* update pcb ptr to new pcb */
	uint32_t* pcb_addr = (uint32_t*) (EIGHT_MB - process_number*EIGHT_KB - EIGHT_KB);
	*((pcb_t*) pcb_addr) = new_pcb;

	cur_pcb = (pcb_t*) pcb_addr;

	/* finally increment process number */
	process_number++;




	/* must save ebp */
	/* prepare for context_switch */
	uint32_t cs = USER_CS;
	uint32_t ds = ((cs & 0x3) == 0) ? KERNEL_DS:USER_DS;
	uint32_t eip = header[24] + (header[25] << 8) + (header[26] << 16) + (header[27] << 24);
	uint32_t esp = v_addr | (FOUR_MB - 4);
	tss.esp0 = EIGHT_MB - process_number*EIGHT_KB;
	register uint32_t ebp asm("ebp");
	cur_pcb->old_ebp = ebp;
	//cur_pcb->old_ebp = asm("ebp");
	asm("											\n\
		pushl	%0				/* push ds	 	*/	\n\
		pushl 	%1				/* push esp  	*/	\n\
		pushf					/* push flags 	*/	\n\
		pushl 	%2				/* push cs 		*/	\n\
		pushl 	%3				/* push eip		*/	\n\
		"
		:
		: "r"(ds), "r"(esp), "r"(cs), "r"(eip)
	);
	/* IRET */
	asm("iret");

	return 0;
};

// System Call 3 - read
/*
 * sys_read_c
 * read data from the keyboard, a file, device (RTC) or directory
 * return the number of bytes read
 */
extern int32_t sys_read_c(int32_t fd, void* buf, int32_t nbytes){
	/* Check validity of inputs */
	if (fd < 0 || fd > FD_SIZE) {
		return -1;
	}
	if (buf == NULL) {
		return -1;
	}
	if (nbytes < 0) {
		return -1;
	}

	// Assert the fd is open
	if ((cur_pcb->file_array)[fd].flags % 2 == 0) {
		return - 1;
	}

	/* call the correct read function with File Operations Jump Table */
	return ((cur_pcb->file_array)[fd].file_op_ptr->read_ptr)(fd, buf, nbytes);
};

// System Call 4 - write
/*
 * sys_write_c
 * writes data to the terminal or to a device (RTC)
 * return the number of bytes written
 */
extern int32_t sys_write_c(int32_t fd, const void* buf, int32_t nbytes){
	/* Check validity of inputs */
	if (fd < 0 || fd > FD_SIZE) {
		return -1;
	}
	if (buf == NULL) {
		return -1;
	}
	if (nbytes < 0) {
		return -1;
	}

	// Assert the fd is open
	if ((cur_pcb->file_array)[fd].flags % 2 == 0) {
		return - 1;
	}

	/* call the correct write function with File Operations Jump Table */
	return ((cur_pcb->file_array)[fd].file_op_ptr->write_ptr)(fd, buf, nbytes);
};

// System Call 5 - open
extern int32_t sys_open_c(const uint8_t* filename){
	dentry_t dentry;
	uint32_t i;

	/* check validity of args */
	if (filename == NULL) {
		return -1;
	}

	/* assert dentry exists */
	if (read_dentry_by_name(filename, &dentry) == -1) {
		return -1;
	}

	/* find next unused file_desc in file array */
	for(i = 2; i < 8; i++) {
		// Check each entry in the file array for an unused entry
		if ((cur_pcb->file_array)[i].flags % 2 == 0){
			break;
		}
	}
	/* assert there was an fd found in the file array */
	if (i >= 8) {
		return -1;
	}

	/* set up correct function table */
	switch (dentry.filetype) {
		case RTC_FILE:
			(cur_pcb->file_array)[i].file_op_ptr = &rtc_table;
			break;
		case DIR_FILE:
			(cur_pcb->file_array)[i].file_op_ptr = &dir_table;
			break;
		case REG_FILE:
			(cur_pcb->file_array)[i].file_op_ptr = &file_table;
			break;
		default:
			(cur_pcb->file_array)[i].file_op_ptr = &terminal_table;
			break;
	}

	/* Call the correct open function with File Operations Jump Table */
	((cur_pcb->file_array)[i].file_op_ptr->open_ptr)(filename);

	/* return the file array index the file was opened in */
	return i;
};

// System Call 6 - Close
extern int32_t sys_close_c(int32_t fd){
	/* assert fd is valid */
	if (fd < 2 || fd > FD_SIZE) {
		return -1;
	}

	// Assert the fd is open
	if ((cur_pcb->file_array)[fd].flags % 2 == 0) {
		return -1;
	}

	// Call the correct close function with File Operations Jump Table
	return ((cur_pcb->file_array)[fd].file_op_ptr->close_ptr)(fd);
};

// System Call 7 - Get Args
extern int32_t sys_getargs_c(uint8_t* buf, int32_t nbytes){
	/*better fix for newline issue, doesn't read newline into the file_name to be searched for*/
	uint32_t in_idx = 0;
	uint32_t idx;
	while ((cur_pcb->input)[in_idx] != ' ') {
		in_idx++;
	}

	in_idx++;

	for (idx = 0; idx < nbytes - in_idx; idx++, in_idx++) {
		buf[idx] = (cur_pcb->input)[in_idx];
		if (buf[idx] == 0) {
			break;
		}
	}

	if (idx == 0) {
		return -1;
	}

	return 0;
};

// System Call 8 - Vidmap
extern int32_t sys_vidmap_c(uint8_t** screen_start){
	/* assert pointer is valid */
	if ((screen_start == NULL) || (screen_start <= (uint8_t**) FOUR_MB)) {
		return -1;
	}

	/* assert page exists */
	if (get_pageTable_entry(USER_VMEM, VID_ADDR >> 12) == NULL) {
		return -1;
	}

	/* alter user pointer */
	*screen_start = (uint8_t*) ((USER_VMEM << 22) | VID_ADDR);

	return 0;
};

// System Call 9 - set_handler
extern int32_t sys_set_handler_c(int32_t signum, void* handler_address){
	return -1;
};

// System Call 10 - sigreturn
extern int32_t sys_sigreturn_c(void){
	return -1;
};
