#ifndef _RTC_DRIVE_H
#define _RTC_DRIVE_H

#include "pcb.h"

// RTC Initialization
void rtc_init();
// RTC Initialization to freq 2048Hz
int rtc_open(const uint8_t* filename);
// DOes nothing
int rtc_close(int32_t fd);
// Change rtc to inputted rate
int rtc_write(int32_t fd, const void* buf, int32_t nbytes);
// Spin until interrupt is received
int rtc_read(int32_t fd, void* buf, int32_t nbytes);
// rtc interrupt handler
extern void rtc_handler();

// interrupt flag
volatile int rtc_interrupt;

//int* rtc_functions[] = {rtc_write, rtc_read};
// RTC MACROS
#define RTC_REGC 0x8C
#define RTC_REGB 0x8B
#define RTC_REGA 0x8A
#define RTC_PORT 0x70
#define RTC_IDT 0x21
#define RTC_6_BIT 0x40
#define top_4 0xF0
#define bot_4 0x0F
#define init_rate 0x0F
#define max_rate  16
#endif
