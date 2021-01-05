#ifndef _KEY_DRIVE_H
#define _KEY_DRIVE_H

#include "types.h"
#define LSHIFT_PRESSED      0x2A
#define LSHIFT_RELEASED	    0xAA
#define RSHIFT_PRESSED	    0x36
#define RSHIFT_RELEASED	    0xB6
#define CTRL_PRESSED        0x1D
#define CTRL_RELEASED       0x9D
#define CAPS_PRESSED        0x3A
#define BACKSPACE           0x0E


#define KEY_BUF_SIZE        128
#define NUM_ROWS            24
#define NUM_COLS            80
#define STDIN               0
#define STDOUT              1
#define NAME_LEN            32

//volatile char[buffer_size] key_buffer;
char key_buf[KEY_BUF_SIZE];

volatile int key_buf_index;
volatile int enter_flag;



volatile unsigned int dict;
volatile int ctrl_mode;
volatile int caps_mode;
volatile int shift_mode;
volatile int mode_flag;


void key_open();

extern void key_handler();

void process_to_buffer(unsigned char input);
void add_to_buffer(char input);
void clear_key_buffer();
void keyboard_environment();
int terminal_open(const uint8_t* filename);
int terminal_close(int32_t fd);
int terminal_read(int32_t fd, void* buf, int32_t nbytes);
int terminal_write(int32_t fd, const void* buf, int32_t nbytes);

// KEY MACROS

#define KEY_PORT 0x60

#endif
