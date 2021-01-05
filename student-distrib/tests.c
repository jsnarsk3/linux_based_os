#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging_c.h"
#include "paging.h"
#include "i8259.h"
#include "filesys.h"
#include "rtc_driver.h"
#include "key_driver.h"
#include "types.h"

#define PASS 1
#define FAIL 0

#define FOUR_KB     0x00001000
#define FOUR_MB     0x00400000

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			//assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Exception Test
 *
 * Tests different exceptions
 * Inputs: None
 * Outputs: None
 * Side Effects: Launches exception handler an freezes kernel
 * Coverage: IDT correctness, Exception Handlers
 * Files: exceptions.S/h, exception_c.h/c
 */
int exception_test(){
	TEST_HEADER;
	// Call the exception
	asm volatile("int $17");
	return 0;
}

/* RTC Test
 *
 * Tests that the RTC is able to send interrupts and that handler can receive interrupts
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints RTC to the screen after each rtc interrupt
 * Coverage: IDT correctness, RTC Handler
 * Files: exceptions.S/h, exception_c.h/c, i8259.c/h
 */
int rtc_test(){
	TEST_HEADER;
	// Unmask the pic ports associated with the rtc
	enable_irq(2);
	enable_irq(8);
	while(1){};
	return 0;
}

/* Keyboard Test
 *
 * Tests that the Keyboard is able to send interrupts and that handler can receive interrupts
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints Keyboard Input to the screen after each Keyboard interrupt
 * Coverage: IDT correctness, Keyboard Handler
 * Files: exceptions.S/h, exception_c.h/c, i8259.c/h
 */
int keyboard_test(){
	TEST_HEADER;
	// Unmask the pic ports associated with the keyboard
	enable_irq(1);
	while(1){};
	return 0;
}
// add more tests here
/* Page Dereferencing Test
 *
 * Check the bounds of the present pages
 * Input: None
 * Output: None
 * Side Effects: None
 * File: paging.h/S, paging_c.h/c
 */
void paging_dereferencing_test() {
	unsigned char* test_addr;
	unsigned char* vidmem_addr;
	unsigned char* kernel_addr;
	unsigned int test_case;

	/* assign base addr for video page and kernel page */
	vidmem_addr = (unsigned char* ) 0x000B8000;
	kernel_addr = (unsigned char* ) 0x00400000;

	/* THIS DETERMINES WHICH TEST CASE TO RUN */
	test_case = 0;

	switch(test_case) {
		case 0:
			/* TEST 0: very start of memory */
			/* SHOULD page fault            */
			test_addr = (unsigned char* ) 0x0;
			break;
		case 1:
			/* TEST 1: boundary of video memory */
			/* SHOULD page fault here */
			test_addr = vidmem_addr - 1;
			break;
		case 2:
			/* TEST 2: at start of vid mem */
			/* SHOULD NOT page fault here */
			test_addr = vidmem_addr;
			break;
		case 3:
			/* TEST 3: at end of vid mem */
			/* SHOULD NOT page fault here */
			test_addr = vidmem_addr + FOUR_KB - 1;
			break;
		case 4:
			/* TEST 4: at boundary of vid mem */
			/* SHOULD page fault here */
			test_addr = vidmem_addr + FOUR_KB;
			break;
		case 5:
			/* TEST 5: at boundary of kernel mem */
			/* SHOULD page fault here */
			test_addr = kernel_addr - 1;
			break;
		case 6:
			/* TEST 6: at start of kernel mem */
			/* SHOULD NOT page fault here */
			test_addr = kernel_addr;
			break;
		case 7:
			/* TEST 7: at end of kernel mem */
			/* SHOULD NOT page fault here */
			test_addr = kernel_addr + FOUR_MB - 1;
			break;
		case 8:
			/* TEST 8: at boundary of kernel mem */
			/* SHOULD page fault here */
			test_addr = kernel_addr + FOUR_MB;
			break;
		default:
			/* default case, but you really shouldn't get here */
			/* SHOULD page fault here */
			test_addr = (unsigned char* ) 0x0;
			break;
	}

	/* will print "oof = " + contents of memory if correct */
	/* else, causes page fault 							   */
	printf("oof = %d\n", *test_addr);
}

/* Paging Test
 *
 * Check if page directory and page table entries hold meaningful values
 * Input: None
 * Output: None
 * Side Effects: None
 * File: paging.h/S, paging_c.h/c
 */
void paging_test() {
	pt_entry_t* test_entry;
	pd_entry_bigPage_t* test_bigPage;

	/* test page table entries */
	test_entry = get_pageTable_entry(0, 0xB8 - 1);
	if (test_entry != NULL) {
		printf("FAIL at page table entries\n");
	}
	test_entry = get_pageTable_entry(0, 0xB8);
	if (test_entry == NULL) {
		printf("FAIL at page table entries\n");
	}
	else {
		if (((test_entry->page_base_addr) << 12) != 0xB8000) {
			printf("FAIL at page table entries\n");
		}
	}
	test_entry = get_pageTable_entry(0, 0xB8 + 1);
	if (test_entry != NULL) {
		printf("FAIL at page table entries\n");
	}
	test_entry = get_pageTable_entry(1, 0);
	if (test_entry != NULL) {
		printf("FAIL at page table entries\n");
	}
	test_entry = get_pageTable_entry(1023, 0);
	if (test_entry != NULL) {
		printf("FAIL at page table entries\n");
	}
	test_entry = get_pageTable_entry(1024, 0);
	if (test_entry != NULL) {
		printf("FAIL at page table entries\n");
	}
	printf("page table entries: PASS\n");

	/* test big pages */
	test_bigPage = get_bigPage(-1);
	if (test_bigPage != NULL) {
		printf("FAIL at page directory entries\n");
	}
	test_bigPage = get_bigPage(1);
	if (test_bigPage == NULL) {
		printf("FAIL at page directory entries\n");
	}
	else {
		if (((test_bigPage->page_base_addr) << 22) != 0x00400000) {
			printf("FAIL at page directory entries\n");
		}
	}
	test_bigPage = get_bigPage(2);
	if (test_bigPage != NULL) {
		printf("FAIL at page directory entries\n");
	}
	test_bigPage = get_bigPage(1023);
	if (test_bigPage != NULL) {
		printf("FAIL at page directory entries\n");
	}
	test_bigPage = get_bigPage(1024);
	if (test_bigPage != NULL) {
		printf("FAIL at page directory entries\n");
	}
	printf("page directory entries: PASS\n");
}


/* Checkpoint 2 tests */
/* RTC Test Driver
 *
 * Tests that the RTC is able to send interrupts and that handler can receive interrupts and whether or not we
 * can change the interrupt rate
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints character to bottom left at each interrupt
 * Coverage: IDT correctness, RTC Handler
 * Files: exceptions.S/h, exception_c.h/c, i8259.c/h
 */
void rtc_test_driver(){
	set_screen(0,0);
	printf("Testing RTC\n");
	// Initialize RTC
	uint8_t in_test[] = "rtc";
	rtc_open(in_test);
	printf("After RTC OPEN, F = 2Hz\n");
	unsigned int count = 30;
	unsigned char rate = 1;
	char rates[] = {9, 12};
	// CHeck each rate
	while(1){
		// Spin until interrupt is received
		rtc_read(0, NULL, 0);
		// Display a character
		putc('1');
		count++;
		if(count > 50){
			count = 0;
			clear();
			set_screen(0,0);
			rate++;
			if(rate == 2){
				rate = 0;
			}
			int frequency = 32000 >> (rates[rate]-1);
			printf("Freqency = %d\n", frequency);
			// Change rate
			if(rtc_write(0, &(rates[rate]), 0) == -1){
				break;
			}
		}
	}
	printf("Invalid Rate Inputted\n");
	printf("%d", rates[rate]);
	while(1){

	}
}



void dir_close_test() {
	int32_t fd;

	for (fd = 0; fd < 9; fd++) {
		if (fd <= 1 || fd >= 8) {
			if (dir_close(fd) == 0) {
				printf("closing unavailable directory %d: FAIL\n", fd);
			}
			else {
				printf("closing unavailable directory %d: PASS\n", fd);
			}
		}
		else {
			if (dir_close(fd) == 0) {
				printf("closing unused directory %d: FAIL\n", fd);
			}
			else {
				printf("closing unused directory %d: PASS\n", fd);
			}
		}
	}
}

void dir_open_test() {
	uint32_t str_length;
	int8_t* temp;
	uint8_t filename[33];
	file_desc_t* file_desc;

	file_desc = get_file_desc();
	printf("init file descriptor flags: 0x%x\n\n", file_desc->flags);

	str_length = strlen(".");
	temp = strncpy((int8_t*) filename, ".", str_length);
	filename[str_length] = 0;
	if (dir_open(filename) == 0) {
		printf("opening directory %s: PASS; ", filename);
		printf("file descriptor flags: 0x%x\n", file_desc->flags);
		if (dir_close(2) == 0) {
			printf("closing directory %s: PASS; ", filename);
		}
		else {
			printf("closing directory %s: FAIL; ", filename);
		}
		printf("file descriptor flags: 0x%x\n", file_desc->flags);
	}
	else {
		printf("opening directory %s: FAIL; ", filename);
		printf("file descriptor flags: 0x%x\n", file_desc->flags);
	}


	str_length = strlen("sigtest");
	temp = strncpy((int8_t*) filename, "sigtest", str_length);
	filename[str_length] = 0;
	if (dir_open(filename) != 0) {
		printf("opening directory %s: PASS; ", filename);
	}
	else {
		printf("opening directory %s: FAIL; ", filename);
	}
	printf("file descriptor flags: 0x%x\n", file_desc->flags);

	str_length = strlen("rtc");
	temp = strncpy((int8_t*) filename, "rtc", str_length);
	filename[str_length] = 0;
	if (dir_open(filename) != 0) {
		printf("opening directory %s: PASS; ", filename);
	}
	else {
		printf("opening directory %s: FAIL; ", filename);
	}
	printf("file descriptor flags: 0x%x\n", file_desc->flags);

	str_length = strlen("verylargetextwithverylongname.tx");
	temp = strncpy((int8_t*) filename, "verylargetextwithverylongname.tx", str_length);
	filename[str_length] = 0;
	if (dir_open(filename) != 0) {
		printf("opening directory large.txt: PASS; ");
	}
	else {
		printf("opening directory large.txt: FAIL; ");
	}
	printf("file descriptor flags: 0x%x\n", file_desc->flags);
}

void dir_read_test() {
	uint32_t str_length;
	int8_t* temp;
	uint8_t filename[33];
	uint8_t buffer[33];
	uint32_t padding;

	dentry_t* read_dentry;

	read_dentry = get_read_dentry();

	str_length = strlen(".");
	temp = strncpy((int8_t*) filename, ".", str_length);
	filename[str_length] = 0;
	if (dir_open((uint8_t*) filename) != 0) {
		printf("dir read test: FAIL; couldn't open directory: %s\n", filename);
		return;
	}

	/* ls */
	printf("ls in directory: .\n");
	while (dir_read(2, buffer, 32) != 0) {
		printf("file_name: ");
		str_length = strlen((int8_t*) buffer);
		padding = 32 - str_length;
		printf("%s", buffer);
		while (padding > 0) {
			printf(" ");
			padding--;
		}
		printf(", file_type: %d, file_size: %d\n", read_dentry->filetype, get_file_size(read_dentry));
	}

	if (dir_close(2) != 0) {
		printf("dir read test: FAIL; couldn't close directory: .\n");
		return;
	}

}

void dir_write_test() {
	if (dir_write(0, NULL, 0) == -1) {
		printf("dir write test: PASS\n");
	}
	else {
		printf("dir write test: FAIL\n");
	}
}

void file_close_test() {
	int32_t fd;

	for (fd = 0; fd < 9; fd++) {
		if (fd <= 1 || fd >= 8) {
			if (file_close(fd) == 0) {
				printf("closing unavailable directory %d: FAIL\n", fd);
			}
			else {
				printf("closing unavailable directory %d: PASS\n", fd);
			}
		}
		else {
			if (file_close(fd) == 0) {
				printf("closing unused directory %d: FAIL\n", fd);
			}
			else {
				printf("closing unused directory %d: PASS\n", fd);
			}
		}
	}
}

void file_open_test() {
	uint32_t str_length;
	int8_t* temp;
	uint8_t filename[33];

	str_length = strlen(".");
	temp = strncpy((int8_t*) filename, ".", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("file open test: FAIL; opened a directory file %s\n", filename);
		return;
	}
	else {
		printf("failed to open directory file %s: PASS\n");
	}


	str_length = strlen("sigtest");
	temp = strncpy((int8_t*) filename, "sigtest", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("shell");
	temp = strncpy((int8_t*) filename, "shell", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("grep");
	temp = strncpy((int8_t*) filename, "grep", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("syserr");
	temp = strncpy((int8_t*) filename, "syserr", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("rtc");
	temp = strncpy((int8_t*) filename, "rtc", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opened an rtc file %s: FAIL\n", filename);
	}
	else {
		printf("failed to open rtc file %s: PASS\n");
	}

	str_length = strlen("fish");
	temp = strncpy((int8_t*) filename, "fish", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("counter");
	temp = strncpy((int8_t*) filename, "counter", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("pingpong");
	temp = strncpy((int8_t*) filename, "pingpong", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("cat");
	temp = strncpy((int8_t*) filename, "cat", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("frame0.txt");
	temp = strncpy((int8_t*) filename, "frame0.txt", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("verylargetextwithverylongname.tx");
	temp = strncpy((int8_t*) filename, "verylargetextwithverylongname.tx", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file large.txt: PASS\n");
	}
	else {
		printf("opening regular file large.txt: FAIL\n");
	}
	if (file_close(2) != 0) {
		printf("could not close file large.txt -> FAIL\n");
	}

	str_length = strlen("ls");
	temp = strncpy((int8_t*) filename, "ls", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("testprint");
	temp = strncpy((int8_t*) filename, "testprint", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("created.txt");
	temp = strncpy((int8_t*) filename, "created.txt", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("frame1.txt");
	temp = strncpy((int8_t*) filename, "frame1.txt", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}

	str_length = strlen("hello");
	temp = strncpy((int8_t*) filename, "hello", str_length);
	filename[str_length] = 0;
	if (file_open(filename) == 0) {
		printf("opening regular file %s: PASS\n", filename);
	}
	else {
		printf("opening regular file %s: FAIL\n", filename);
	}
	if (file_close(2) != 0) {
		printf("could not close file %s -> FAIL\n", filename);
	}
}

void file_read_test() {
	uint32_t ret_val;
	uint32_t str_length;
	int8_t* temp;
	uint8_t buffer[33];
	uint8_t filename[33];
	int count = 0;

	str_length = strlen("hello");
	temp = strncpy((int8_t*) filename, "hello", str_length);
	filename[str_length] = 0;

	if (file_open(filename) != 0) {
		printf("file read test: FAIL; failed opening %s\n", (uint8_t*) filename);
		return;
	}

	while (0 != (ret_val = file_read(2, buffer, 32))) {
		printf("%s", buffer);
		count += ret_val;
	}
	printf("file read test: PASS; count = %d\n", count);

	if (file_close(3) != 0) {
		printf("file read test: FAIL; failed closing %s\n", (uint8_t*) filename);
		return;
	}
}

void file_write_test() {
	if (file_write(0, NULL, 0) == -1) {
		printf("file write test: PASS\n");
	}
	else {
		printf("file write test: FAIL\n");
	}
}

void terminal_tests(){
    terminal_open(0);
	char input[129];
	while(1){
	terminal_read(0, input, 128);
	terminal_write(0, input, 128);
	}
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
 	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("rtc test", rtc_test());
	//TEST_OUTPUT("keyboard test", keyboard_test());
	//TEST_OUTPUT("Exception Test", exception_test());

	//paging_dereferencing_test();
	//paging_test();

	clear();
//	rtc_test_driver();
//	terminal_tests();
//	dir_close_test();
//	dir_open_test();
	dir_read_test();
//	dir_write_test();
//	file_close_test();
//	file_open_test();
//	file_read_test();
// 	file_write_test();
	while(1){}
}
