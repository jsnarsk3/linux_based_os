#ifndef PCB_H_
#define PCB_H_

#include "types.h"

typedef struct fd_ops {
    int32_t (*open_ptr)(const uint8_t*);
    int32_t (*close_ptr)(int32_t);
    int32_t (*read_ptr)(int32_t, void*, int32_t);
    int32_t (*write_ptr)(int32_t, const void*, int32_t);
} fd_ops_t;

typedef struct file_desc {
    fd_ops_t* file_op_ptr;      /* file operations jmp table associated with file type  */
    uint32_t inode;             /* inode number for this file                           */
    uint32_t file_pos;          /* file position                                        */
    uint32_t flags;             /* Flags:   [0]:    in use (1), not in use (0)          */
                                /*          [2:1]:  filetype (0, 1, 2)                  */
                                /*          [19:8]: index in inode (data block number)  */
                                /*          [31:20] offset                              */
} file_desc_t;

typedef struct pcb {
    file_desc_t file_array[8];
    struct pcb_t* old_pcb_ptr;
    uint8_t input[1024];
    uint32_t old_phys_addr;
    uint32_t old_esp0;
    uint32_t old_ebp;
} pcb_t;

pcb_t* cur_pcb;

#endif
