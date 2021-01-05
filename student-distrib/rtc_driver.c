#include "rtc_driver.h"
#include "i8259.h"
#include "lib.h"
#include "pcb.h"

/* void rtc_init
 *   Inputs: None
 *   Outputs: None
 *   Function: Sets the registers in RTC to initialize it
*/
void rtc_init(){
	outb(RTC_REGB, RTC_PORT);
	char prev=inb(RTC_PORT+1);
	outb(RTC_REGB, RTC_PORT);
	outb( prev | RTC_6_BIT, RTC_PORT+1);
};

/* void rtc_open
 *   Inputs: None
 *   Outputs: Returns zero upon successful initialization
 *   Function: Initializes rtc to 2khz
*/
int rtc_open(const uint8_t* filename){
	// initialize rtc frequency to 2Hz;
	unsigned char rate = init_rate;			// rate must be above 2 and not over 15
	cli();
	outb(RTC_REGA, RTC_PORT);		// set index to register A, disable NMI
	char prev=inb(RTC_PORT+1);	// get initial value of register A
	outb(RTC_REGA, RTC_PORT);		// reset index to A
	outb((prev & top_4) | rate, RTC_PORT+1); //write only our rate to A. Note, rate is the bottom 4 bits.
	// Unmask the pic ports associated with the rtc
	enable_irq(2);
	enable_irq(8);
	rtc_interrupt = 0;
	// Add rtc to the file array
	// Find an empty file descriptor
	int i, file_idx;
	file_idx = -1;
	for(i = 2; i<8;i++){
		// Check each entry in the file array for an unused entry
		if((cur_pcb->file_array)[i].flags % 2 == 0){
			file_idx = i;
			break;
		}
	}
	// No empty file descriptor found
	if (file_idx == -1){
		return -1;
	}

	(cur_pcb->file_array)[file_idx].inode = 0;        // set inode num
    (cur_pcb->file_array)[file_idx].file_pos = 0;                         // start at oth byte
    (cur_pcb->file_array)[file_idx].flags = 0x00000001;                   // mark as in use
    (cur_pcb->file_array)[file_idx].flags |= 0 << 1;      // mark as rtc type
    (cur_pcb->file_array)[file_idx].flags |= 0x1 << 8;
	sti();
	return 0;
};

/* void rtc_close
 *   Inputs: None
 *   Outputs: returns 0
 *   Function: does nothing
*/
int rtc_close(int32_t fd){
	// Probably does nothing
	/* bounds check file directory */
    if (fd <= 1 || fd >= 8) {
        return -1;
    }

	if ((((cur_pcb->file_array)[fd].flags & 0x6) >> 1) != 0) {
        return -1;
    }

	/* assert there is an open file_desc */
    if ((cur_pcb->file_array)[fd].flags % 2 == 0) {
        return -1;
    }

    /* denote not in use and return success */
   	(cur_pcb->file_array)[fd].flags = 0;
	return 0;
};

/* void rtc_write
 *   Inputs: unsigned char rate - rate to set rtc
 *   Outputs: returns 0 - successfully wrote new frequency
 *			  returns -1 - incorrect rate
 *   Function: Sets the frequency of the rtc to the given rate
*/
int rtc_write(int32_t fd, const void* buf, int32_t nbytes){
	unsigned char* rate_ptr = (unsigned char*)buf;
	unsigned char rate = *rate_ptr;
	uint32_t input_rate = (uint32_t) rate;
	uint32_t input_base = 0;

	/* determine that rate is a power of two */
	if ((input_rate & (input_rate - 1)) != 0) {
		return -1;
	}
	/* find base 2 power of input rate and assert less than 1024Hz */
	while (input_rate >>= 1) {
		input_base++;
	}
	if ((input_base < 2) || (input_base > 10)){
		return -1;
	}

	// Changes frequency
	cli();
	outb(RTC_REGA, RTC_PORT);		// set index to register A, disable NMI
	char prev=inb(RTC_PORT | 0x1);	// get initial value of register A
	outb(RTC_REGA, RTC_PORT);		// reset index to A
	outb((prev & top_4) | ((max_rate - input_base) & bot_4), RTC_PORT | 0x1); //write only our rate to A. Note, rate is the bottom 4 bits.
	sti();
	return 0;
};

/* void rtc_read
 *   Inputs: None
 *   Outputs: returns 0 upon receiving an interrupt
 *   Function: Spins until an rtc interrupt is received
*/
int rtc_read(int32_t fd, void* buf, int32_t nbytes){
	// Wait for an interrupt
	while(1){
		// if interrupt happens
		cli();
		if(rtc_interrupt){
			rtc_interrupt = 0;
			break;
		}
		sti();
	}
	// Service Interrupt
	return 0;
}
/* void rtc_handler();
 * Inputs: none
 * Return Value: none
 * Function: Handles an RTC interrupt and prints RTC to the screen */
extern void rtc_handler() {
	cli();
	// Receive and service the interrupt
	rtc_interrupt = 1;
	outb(RTC_REGC,RTC_PORT);
	inb(RTC_PORT+1);
	send_eoi(8);
	sti();
}
