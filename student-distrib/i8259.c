/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*	i8259_init()
 *	Initialize the 8259 PIC 
 *
 *	inputs: none
 *	outputs: none
 *	side effects: Sends the correct control words to the PIC to intialize it
*/
void i8259_init(void) {
	/*first, disable interrupts on the processer 
	and mask the interrupts on the master and slave PIC*/
	//outb(0xFF, MASTER_8259_PORT);
	//outb(0xFF, SLAVE_8259_PORT);

	/*set up the master PIC*/
	/*four initialiazing bytes to send
	*/
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT+1);  // map interrupts on IRQs to IDT vectors 0x20-0x27
	outb(ICW3_MASTER, MASTER_8259_PORT+1);
	outb(ICW4, MASTER_8259_PORT+1);

	/*initialize the slave PIC, four initializing bytes*/
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT+1);  //maps interrupts on slave IRQs to IDT vectors 0x28-0x2f
	outb(ICW3_SLAVE, SLAVE_8259_PORT+1);
	outb(ICW4, SLAVE_8259_PORT+1);

	/*reenable interrupts on the processer*/
}

/*	enable_irq
 *	Enable (unmask) the specified IRQ
 *
 *	inputs: uint32_t irq_num : irq number of the pin on the pic to unmask
 *	outputs: none
 *	side effects: unmasks the specified interrupt on the pic
*/
void enable_irq(uint32_t irq_num) {
	uint16_t port;
    uint8_t value;
 
    if(irq_num < 8) {
		value = master_mask & ~(1 << irq_num);
		master_mask = value;
		port = MASTER_8259_PORT+1;
    } else {
        irq_num -= 8;
		value = slave_mask & ~(1 << irq_num);
		slave_mask = value;
		port = SLAVE_8259_PORT+1;
    }
    outb(value, port);  
}

/*	disable_irq
 *	Disable (mask) the specified IRQ
 *
 *	inputs: uint32_t irq_num : irq number of the pin on the pic to mask
 *	outputs: none
 *	side effects: masks the specified interrupt on the pic
*/
void disable_irq(uint32_t irq_num) {
	uint16_t port;
    uint8_t value;
 
    if(irq_num < 8) {
		value = master_mask | (1 << irq_num);
		master_mask = value;
		port = MASTER_8259_PORT+1;
    } else {
        irq_num -= 8;
		value = slave_mask | (1 << irq_num);
		slave_mask = value;
		port = SLAVE_8259_PORT+1;
    }
    outb(value, port); 
}

/*	send_eoi
 *	Send end-of-interrupt signal for the specified IRQ
 *
 *	inputs: uint32_t irq_num : irq number that interrupt signal originated from
 *	outputs: none
 *	side effects: sends end of interrupt signal to master pic and also slave pic if needed
*/
void send_eoi(uint32_t irq_num) {
	if(irq_num >= 8){
		outb(EOI, SLAVE_8259_PORT);
	}
	outb(EOI, MASTER_8259_PORT);
}
