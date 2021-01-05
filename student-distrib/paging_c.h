#ifndef _PAGING_C_H
#define _PAGING_C_H

#include "types.h"
#include "x86_desc.h"

#define FOUR_MB     0x00400000
#define FOUR_KB     0x00001000
#define EIGHT_KB    0x00002000
#define EIGHT_MB    0x00800000
#define THTWO_MB    0x02000000

#define VID_ADDR    0x000B8000
#define TWENTY_MSB  0xFFFFF000
#define KER_ADDR    FOUR_MB
#define TEN_MSB     0xFFC00000
#define PRO_ADDR    32

#define USER_VMEM   33

/* struct for holding pages in page table */
typedef struct pd_entry_smallPage {
    uint32_t present                : 1;        /* denotes if entry is in physical memory (1) */
    uint32_t rw_enable              : 1;        /* 0:read only, 1:r/w enabled                 */
    uint32_t user_super             : 1;        /* sets privileges                            */
    uint32_t write_through          : 1;        /* 0:disabled, 1:enabled                      */
    uint32_t cache_disabled         : 1;        /* controls caching; 0:enabled. 1:disabled    */
    uint32_t accessed               : 1;        /* indicates if was accessed (r/w) when set   */
    uint32_t reserved               : 1;        /* set to 0                                   */
    uint32_t page_size              : 1;        /* 0 indicates 4 KB                           */
    uint32_t global_page            : 1;        /* (ignored)                                  */
    uint32_t available              : 3;        /* available for system programmer's use      */
    uint32_t page_table_base_addr   : 20;       /* base address of page table                 */

} pd_entry_smallPage_t;

/* struct for a 4MB page entry in the page directory */
typedef struct pd_entry_bigPage {
    uint32_t present                : 1;        /* denotes if entry is in physical memory (1) */
    uint32_t rw_enable              : 1;        /* 0:read only, 1:r/w enabled                 */
    uint32_t user_super             : 1;        /* sets privileges                            */
    uint32_t write_through          : 1;        /* 0:disabled, 1:enabled                      */
    uint32_t cache_disabled         : 1;        /* controls caching; 0:enabled. 1:disabled    */
    uint32_t accessed               : 1;        /* indicates if was accessed (r/w) when set   */
    uint32_t dirty                  : 1;        /* indicates if was written to when set       */
    uint32_t page_size              : 1;        /* 0:4KB, 1:4MB                               */
    uint32_t global_page            : 1;        /* if set, do not invalidate when TLB flushed */
    uint32_t available              : 3;        /* available for system programmer's use      */
    uint32_t pat_idx                : 1;        /* page attribute table index; sets mem type  */
    uint32_t reserved               : 9;        /* reserved (do not use)                      */
    uint32_t page_base_addr         : 10;       /* base address of page                       */
} pd_entry_bigPage_t;

/* struct for holding either a 4kB page or a 4MB, to allow for use with page_directory[1024] */
typedef union pd_entry {
    pd_entry_smallPage_t smallPage;             /* holds small page in the entry              */
    pd_entry_bigPage_t bigPage;                 /* holds big page in the entry                */
} pd_entry_t;

/* structure for a 4kB page entry in the page table */
typedef struct pt_entry {
    uint32_t present                : 1;        /* denotes if entry is in physical memory (1) */
    uint32_t rw_enable              : 1;        /* 0:read only, 1:r/w enabled                 */
    uint32_t user_super             : 1;        /* sets privileges                            */
    uint32_t write_through          : 1;        /* 0:disabled, 1:enabled                      */
    uint32_t cache_disabled         : 1;        /* controls caching; 0:enabled. 1:disabled    */
    uint32_t accessed               : 1;        /* indicates if was accessed (r/w) when set   */
    uint32_t dirty                  : 1;        /* indicates if was written to when set       */
    uint32_t pat_idx                : 1;        /* page attribute table index; sets mem type  */
    uint32_t global_page            : 1;        /* if set, do not invalidate when TLB flushed */
    uint32_t available              : 3;        /* available for system programmer's use      */
    uint32_t page_base_addr         : 20;       /* base address of page                       */
} pt_entry_t;

/* initializes paging */
void init_paging(void);

/* returns pointer to a page table entry (4kB page) */
pt_entry_t* get_pageTable_entry(uint32_t dir_idx, uint32_t table_idx);

/* returns pointer to a page directory entry (4MB page) */
pd_entry_bigPage_t* get_bigPage(uint32_t dir_idx);

#endif
