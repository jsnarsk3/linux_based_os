#ifndef _PAGING_H
#define _PAGING_H

#ifndef ASM

/* loads ptr to page directory into cr3 */
extern void lpdt(void* page_dir);

/* sets paging bit in cr0 */
extern void enablePaging(void);

/* sets paging size extension bit in cr4 */
extern void enablePSE(void);

/* sets paging global extension bit in cr4 */
extern void enablePGE(void);

/* returns pointer to page directory */
extern void* ret_dir_ptr(void);

#endif /* ASM */
#endif /* _PAGING_H */
