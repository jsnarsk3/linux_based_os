#include "key_driver.h"
#include "i8259.h"
#include "lib.h"
#include "pcb.h"

/*this array holds the correct keys for the normal input, shifted input, and capslock input
*/
unsigned char key_array[3][128] = {
	//first, normal key board
	{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
},



	//then keyboard with shift
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '\'', '~',   0,		/* Left shift */
 '\\', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
},


	//then keyboard with caps lock
	{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
}
};


/* void keyboard_handler();
 * Inputs: none
 * Return Value: none
 * Function: handles keyboard interrupt to display on the console */
void keyboard_environment(){
	key_open();
	//clear();
	//set_screen(0,0);
	dict = 0;
	caps_mode = 0;
	shift_mode = 0;
	ctrl_mode = 0;
}


/* void key_open();
 * Inputs: none
 * Return Value: none
 * Function: unmasks the keyboard interrupts on the pic */
void key_open(){
	enable_irq(1);
}


/* void key_handler();
 * Inputs: none
 * Return Value: none
 * Function: handles keyboard interrupt to display on the console */
extern void key_handler(){
	// Print Data From Keyboard
	cli();
	unsigned char input = inb(KEY_PORT);
	unsigned char symbol = key_array[dict][input];


	//putc(input);
	//putc(symbol);
	enter_flag = 0;
	mode_flag = 0;

	/* handle left and right shift keys */
	if (input == RSHIFT_PRESSED || input == LSHIFT_PRESSED) {
		shift_mode = 1;
		mode_flag = 1;
	}
	else if (input == RSHIFT_RELEASED || input == LSHIFT_RELEASED) {
		shift_mode = 0;
		mode_flag = 1;
	}

	if (input == CAPS_PRESSED) {
		caps_mode = ~caps_mode;
		mode_flag = 1;
	}

	/* handle CTRL */
	if (input == CTRL_PRESSED){
		ctrl_mode = 1;
		mode_flag = 1;
	}
	else if (input == CTRL_RELEASED){
		ctrl_mode = 0;
		mode_flag = 1;
	}

	/* change dictionary according to shift and caps combo */
	if (shift_mode && caps_mode) {
		dict = 0;
	}
	else if (shift_mode && (~caps_mode)) {
		dict = 1;
	}
	else if ((~shift_mode) && caps_mode) {
		dict = 2;
	}
	else {
		dict = 0;
	}

	if (input >= 0x80 || mode_flag) {
		send_eoi(1);
		sti();
	}

	/*took care of backspace here, because it was having trouble when i called a separate function to handle backspace
	when the keyboard reads a backspace, it deletes the previous character in the key_buffer
	it also deletes the previous character on the screen itself by working with video memory*/
	else if(symbol == '\b'){
		if(key_buf_index>0){
			deletec_terminal(key_buf[key_buf_index -1]);
			key_buf[key_buf_index] = 0;
			key_buf_index -= 1;
		}
	}




	/*after taking care of the special cases, print the character to the screen */
	else {
		if (symbol == '\n'){
			enter_flag = 1;
		}
		process_to_buffer(input);
	}

	send_eoi(1);
	sti();
}



/* void process_to_buffer;
 * Inputs: a character to be writtten
 * Return Value: none
 * Function: makes the character display on the screen correctly, and adds the character to the key_buffer */
void process_to_buffer(unsigned char input) {
//	putc(input);
	unsigned char symbol = key_array[dict][input];

	if (ctrl_mode) {
		if (symbol == 'l' || symbol == 'L'){
			clear();
			set_screen(0,0);
		}
	}
	else if (key_buf_index < KEY_BUF_SIZE) {
		putc_terminal(symbol);
		add_to_buffer(symbol);
	}
}



/* void add_to_buffer();
 * Inputs: char to be added to the key buffer
 * Return Value: none
 * Function: handles keyboard interrupt to display on the console */
void add_to_buffer(char symbol){
	if (key_buf_index < KEY_BUF_SIZE){
		key_buf[key_buf_index] = symbol;
		key_buf_index += 1;
	}
}



/* void clear_key_buffer();
 * Inputs: none
 * Return Value: none
 * Function: clears the key_buffer and resets the key buffer index */
void clear_key_buffer(){
	int i;
	for (i = 0; i < KEY_BUF_SIZE; i++){
		key_buf[i] = 0;
	}
	key_buf_index = 0;
}


/* int terminal_open();
 * Inputs: pointer to a uint8, a filename
 * Return Value: 0 on success
 * Function: sets up use of keyboard and of terminal read and write functions */
int terminal_open(const uint8_t* filename) {
    keyboard_environment();
	return 0;
}


/* int terminal_close();
 * Inputs: int32t, a file descriptor
 * Return Value: 0 on success
 * Function: nothingg really*/
int terminal_close(int32_t fd) {
	/* bounds check file directory */
    if (fd < 0 || fd > 1) {
        return -1;
    }

	/* assert there is an open file_desc */
	if ((cur_pcb->file_array)[fd].flags % 2 == 0) {
		return -1;
	}

	/* denote not in use and return success */
	(cur_pcb->file_array)[fd].flags = 0;
	return 0;
}


/* int terminal_read();
 * Inputs: 3 standard parameters: a file descriptor, a buffer, and a number of bytes to be read
 * Return Value: the number of bytes read
 * Function: reads from the keyboard_buffer and moves the contents into the buffer passed by the caller*/
int terminal_read(int32_t fd, void* buf, int32_t nbytes) {
	//clear_key_buffer();
	uint32_t i;
	int8_t* buffer = (int8_t*) buf;

	/* assert can only write to stdout */
	if (fd != STDIN) {
		return -1;
	}



	//wait for enter key to be pressed, when it is pressed, you should copy the key buffer into the passed in location
	clear_key_buffer();
	while (1) {
		if (enter_flag == 1) {

			for (i = 0; i < key_buf_index; i++) {
				buffer[i] = key_buf[i];
			}
			break;
		}
	}
	enter_flag = 0;
	clear_key_buffer();
	return i;
}


/* int terminal_write();
 * Inputs: file descriptor, buffer, number of bytes to be written
 * Return Value: number of bytes written
 * Function: writes from the buffer passed in, to the screen*/
int terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
	int32_t index;
	int8_t* inbuf = (int8_t*) buf;

	/* assert can only write to stdout */
	if (fd != STDOUT) {
		return -1;
	}

	cli();
	index = 0;
	while (index < nbytes) {
		putc_terminal(inbuf[index]);
		index++;
	}
	sti();

	return index;
}
