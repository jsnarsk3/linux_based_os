#ifndef SYS_CALLS_H_
#define SYS_CALLS_H_

#define ELF         0x7F
#define PRO_IDX     32
#define EIGHT_MASK  0xFFC00000
#define EIGHT_S     22
#define USER_PROG   0x8048000

#include "x86_desc.h"
#include "rtc_driver.h"
#include "filesys.h"
#include "key_driver.h"
#include "paging_c.h"
#include "paging.h"
#include "types.h"
#include "lib.h"

#ifndef ASM

file_desc_t clear_fd;
uint32_t process_number = 0;
int8_t elf_text[4] = {ELF, 'E', 'L', 'F'};

fd_ops_t rtc_table = {&rtc_open, &rtc_close, &rtc_read, &rtc_write};
fd_ops_t dir_table = {&dir_open, &dir_close, &dir_read, &dir_write};
fd_ops_t file_table = {&file_open, &file_close, &file_read, &file_write};
fd_ops_t terminal_table = {&terminal_open, &terminal_close, &terminal_read, &terminal_write};


// System Call 1 - Halt
extern int32_t sys_halt_c(uint8_t status);
// System Call 2 - Execute
extern int32_t sys_execute_c(const uint8_t* command);
// System Call 3 - read
extern int32_t sys_read_c(int32_t fd, void* buf, int32_t nbytes);
// System Call 4 - write
extern int32_t sys_write_c(int32_t fd, const void* buf, int32_t nbytes);
// System Call 5 - open
extern int32_t sys_open_c(const uint8_t* filename);
// System Call 6 - Close
extern int32_t sys_close_c(int32_t fd);
// System Call 7 - Get Args
extern int32_t sys_getargs_c(uint8_t* buf, int32_t nbytes);
// System Call 8 - Vidmap
extern int32_t sys_vidmap_c(uint8_t** screen_start);
// System Call 9 - set_handler
extern int32_t sys_set_handler_c(int32_t signum, void* handler_address);
// System Call 10 - sigreturn
extern int32_t sys_sigreturn_c(void);


#endif
#endif
