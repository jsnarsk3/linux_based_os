#define ASM     1
#include "paging.h"

.text

.globl lpdt
.globl enablePaging
.globl enablePSE
.globl enablePGE
.globl ret_dir_ptr

.align 4

pg_flag:
    .long   0x80000000
pg_base_mask:
    .long   0xFFFFF000
pg_msb_mask:
    .long   0x00000FFF


pse_flag:
    .long   0x00000010

pge_flag:
    .long   0x00000080

# void lpdt(void);
# loads ptr to page directory into cr3
# inputs: none
# outputs: none
# side effects: flushes TLB and links page directory
lpdt:
    push    %ebp                    # save old base ptr
    movl    %esp, %ebp              # set new base ptr
    movl    8(%ebp), %eax           # eax <- page_directory
    movl    %cr3, %edx              # edx <- cr3
    and     pg_base_mask, %eax      # mask out flag bits in page_dir addr
    and     pg_msb_mask, %edx       # clear out addr bits in cr3
    orl     %edx, %eax              # combine eax and edx
    movl    %eax, %cr3              # cr3 <- page_directory
    leave                           # leave and ret
    ret                             #

# void enablePaging(void);
# sets paging bit in cr0
# inputs: none
# outputs: none
# side effects: enables paging
enablePaging:
    push    %ebp                    # save old base ptr
    movl    %esp, %ebp              # set new base ptr
    movl    %cr0, %eax              # get current cr0
    orl     pg_flag, %eax           # set bit 31 of cr0 to 1, which enables paging
    movl    %eax, %cr0              # restore cr0 with paging enabled
    leave                           # leave and ret
    ret                             #

# void enablePSE(void);
# sets paging size extension bit in cr4
# inputs: none
# outputs: none
# side effects: enables page size extensions
enablePSE:
    push    %ebp                    # save old base ptr
    movl    %esp, %ebp              # set new base ptr
    movl    %cr4, %eax              # get current cr4
    orl     pse_flag, %eax          # set bit 4 of cr4 to 1, which enables page size extensions
    movl    %eax, %cr4              # restore cr4 with paging enabled
    leave                           # leave and ret
    ret                             #

# void enablePGE(void);
# sets paging global extension bit in cr4
# inputs: none
# outputs: none
# side effects: enables global pages
enablePGE:
    push    %ebp                    # save old base ptr
    movl    %esp, %ebp              # set new base ptr
    movl    %cr4, %eax              # get current cr4
    orl     pge_flag, %eax          # set bit 4 of cr4 to 1, which enables page size extensions
    movl    %eax, %cr4              # restore cr4 with paging enabled
    leave                           # leave and ret
    ret

# void* ret_dir_ptr(void);
# returns ptr to page directory
# inputs: none
# outputs: none
# side effects: clobbers eax
ret_dir_ptr:
    push    %ebp                    # save old base ptr
    movl    %esp, %ebp              # set new base ptr
    movl    %cr3, %eax              # get current cr4
    andl    pg_base_mask, %eax      # mask lower bits s.t. only address is returned from cr3
    leave                           # leave and ret
    ret
