#ifndef _FILESYS_H
#define _FILESYS_H

#include "pcb.h"
#include "types.h"
#include "lib.h"
#include "pcb.h"


#define FOUR_KB         0x00001000
#define NAME_LEN        32
#define RES_LEN         24
#define DENTRY_SIZE     64

#define RTC_FILE        0
#define DIR_FILE        1
#define REG_FILE        2

#define FN_OFFSET       8
#define FT_OFFSET       9

#define B_ZERO_MASK     0x000000FF
#define B_ONE_MASK      0x0000FF00
#define B_TWO_MASK      0x00FF0000
#define B_THREE_MASK    0xFF000000

#define USE_MASK        0x00000001
#define TYPE_MASK       0x00000006
#define I_IDX_MASK      0x000FFF00
#define I_IDX_CLEAR     0xFFF000FF

#define TYPE_SHIFT      1
#define I_IDX_SHIFT     8
#define OFFSET_SHIFT    20

/* boot block information, aligned to 4kB */
typedef struct bblock {
    uint32_t n_dir_entries;     /* number of directory entries; 4B      */
    uint32_t N;                 /* number of inodes;            4B      */
    uint32_t D;                 /* number of data blocks;       4B      */
    uint32_t reserved;          /* reserved;                    4B      */
} bblock_t;

/* inode infromation, aligned to 4kB */
typedef struct inode {
  uint32_t length;              /* length of the file; 4B    */
  uint32_t data_block_arr[1023];/* array for the data blocks */
} inode_t;

/* directory entry block, aligned to 4kB */
typedef struct dentry {
    uint8_t filename[32];       /* filename;        32B */
    uint32_t filetype;          /* type of file;    4B  */
    uint32_t inode_num;         /* inode number;    4B  */
    uint8_t reserved[24];       /* reserved;        24B */
} dentry_t;


/* initializes filesystem at specified address */
void init_filesys(uint32_t* ptr);

/* testing functions */
/* return structs (for testing) */
dentry_t* get_read_dentry();
dentry_t* get_open_dentry();
file_desc_t* get_file_desc();
/* return filesize of input dentry (for testing) */
int32_t get_file_size(dentry_t* dentry);

/* file system routines */
/* copies dentry from src to dest by name */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
/* copies dentry from src to dest by index in boot block */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
/* copies specified number of bytes from data block to buffer */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length, int32_t fd);


/* file driver functions */
/* read operation for regular files */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes);
/* write operation for regular files */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);
/* open operation for regular files */
int32_t file_open (const uint8_t* filename);
/* close operation for regular files */
int32_t file_close (int32_t fd);


/* directory driver functions */
/* read operation for directory files */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);
/* write operation for directory files */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);
/* open operation for directory files */
int32_t dir_open (const uint8_t* filename);
/* close operation for directory files */
int32_t dir_close (int32_t fd);

#endif /* _FILESYS_H */
