#include "paging_c.h"
#include "paging.h"

/* declare page directory and page table */
static pd_entry_t page_directory[1024] __attribute__((aligned(FOUR_KB)));
static pt_entry_t page_table_0[1024] __attribute__((aligned(FOUR_KB)));
static pt_entry_t page_table_1[1024] __attribute__((aligned(FOUR_KB)));

/* clear page directory table entries, for initialization */
pd_entry_t pd_clear_entry;
pd_entry_bigPage_t clearBigPage;
pt_entry_t pt_clear_entry;

/* page_directory[0] -> page_0: small page, contains video memory */
pt_entry_t vid_mem;
pd_entry_smallPage_t pt_0;
pd_entry_t pd_entry_0;

/* page_directory[1] -> page_1: big page, contains kernel */
pd_entry_bigPage_t kernel_page;
pd_entry_t prog;
pd_entry_t pd_entry_1;

/* page_directory[32] -> program page, contains programs loaded for execute */
pd_entry_bigPage_t program_page;

/* page_directory[2] -> page */
pt_entry_t program_vmem;
pd_entry_smallPage_t pt_vmem;
pd_entry_t pd_vmem;

/*
 * init_paging
 *   DESCRIPTION: initializes paging; creates a small page for the video memory and
 *                and a big page for the kernel memory
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modifies registers cr0, cr3, cr4
 */
void init_paging(void) {
    uint32_t it;
    clearBigPage.present = 0;
    pd_clear_entry.bigPage = clearBigPage;
    pt_clear_entry.present = 0;

    /* init page directory */
    for (it = 0; it < 1024; it++) {
        /* set to not present (bit 0), write enabled (bit 1), user (bit 2) */
        page_directory[it] = pd_clear_entry;
    }

    /* init page_table_0 */
    for (it = 0; it < 1024; it++) {
        /* set to not present (bit 0), write enabled (bit 1),  and user (bit 2) */
        page_table_0[it] = pt_clear_entry;
    }

    /* construct page table entry for video memory */
    vid_mem.present = 1;
    vid_mem.rw_enable = 1;
    vid_mem.user_super = 0;
    vid_mem.write_through = 0;
    vid_mem.cache_disabled = 0;
    vid_mem.accessed = 0;
    vid_mem.dirty = 0;
    vid_mem.pat_idx = 0;
    vid_mem.global_page = 0;
    vid_mem.available = 0;
    vid_mem.page_base_addr = (uint32_t) ((TWENTY_MSB & VID_ADDR) >> 12);
    /* place video memory entry int page table 0 */
    page_table_0[(TWENTY_MSB & VID_ADDR) >> 12] = vid_mem;

    /* construct small page entry for page table 0 (0MB to 4MB) */
    pt_0.present = 1;
    pt_0.rw_enable = 1;
    pt_0.user_super = 0;
    pt_0.write_through = 0;
    pt_0.cache_disabled = 0;
    pt_0.accessed = 0;
    pt_0.reserved = 0;
    pt_0.page_size = 0;
    pt_0.global_page = 0;
    pt_0.available = 0;
    pt_0.page_table_base_addr = (uint32_t) ((TWENTY_MSB & ((uint32_t) page_table_0)) >> 12);
    /* create page directory entry of smallPage type */
    pd_entry_0.smallPage = pt_0;
    /* place page table 0 into page directory */
    page_directory[0] = pd_entry_0;

    /* construct page directory entry for kernel */
    kernel_page.present = 1;
    kernel_page.rw_enable = 1;
    kernel_page.user_super = 0;
    kernel_page.write_through = 0;
    kernel_page.cache_disabled = 1;
    kernel_page.accessed = 0;
    kernel_page.dirty = 0;
    kernel_page.page_size = 1;
    kernel_page.global_page = 1;
    kernel_page.available = 0;
    kernel_page.pat_idx = 0;
    kernel_page.reserved = 0;
    kernel_page.page_base_addr = (uint32_t) ((TEN_MSB & KER_ADDR) >> 22);
    /* construct page directory entry of bigPage type */
    pd_entry_1.bigPage = kernel_page;
    /* place big page into page directory */
    page_directory[1] = pd_entry_1;

    /* create page for user level programs */
    program_page.present = 1;
    program_page.rw_enable = 1;
    program_page.user_super = 1;
    program_page.write_through = 0;
    program_page.cache_disabled = 1;
    program_page.accessed = 0;
    program_page.dirty = 0;
    program_page.page_size = 1;
    program_page.global_page = 0;
    program_page.available = 0;
    program_page.pat_idx = 0;
    program_page.reserved = 0;
    program_page.page_base_addr = 0;
    prog.bigPage = program_page;
    page_directory[PRO_ADDR] = prog;

    /* construct page for program vmem */
    program_vmem.present = 1;
    program_vmem.rw_enable = 1;
    program_vmem.user_super = 1;
    program_vmem.write_through = 0;
    program_vmem.cache_disabled = 0;
    program_vmem.accessed = 0;
    program_vmem.dirty = 0;
    program_vmem.pat_idx = 0;
    program_vmem.global_page = 0;
    program_vmem.available = 0;
    program_vmem.page_base_addr = (uint32_t) ((TWENTY_MSB & VID_ADDR) >> 12);
    page_table_1[(TWENTY_MSB & VID_ADDR) >> 12] = program_vmem;

    pt_vmem.present = 1;
    pt_vmem.rw_enable = 1;
    pt_vmem.user_super = 1;
    pt_vmem.write_through = 0;
    pt_vmem.cache_disabled = 0;
    pt_vmem.accessed = 0;
    pt_vmem.reserved = 0;
    pt_vmem.page_size = 0;
    pt_vmem.global_page = 0;
    pt_vmem.available = 0;
    pt_vmem.page_table_base_addr = (uint32_t) ((TWENTY_MSB & ((uint32_t) page_table_1)) >> 12);

    pd_vmem.smallPage = pt_vmem;
    page_directory[USER_VMEM] = pd_vmem;

    /* create page directory entry of smallPage type */
    pd_entry_0.smallPage = pt_0;
    /* place page table 0 into page directory */
    page_directory[0] = pd_entry_0;
    /* load page dir p=tr into pdtr (cr3) */
    lpdt((void*) page_directory);
    /* enable page size extensions (allows for mixing of 4KB and 4MB pages) */
    enablePSE();
    /* enable global pages */
    enablePGE();
    /* enable paging (msb cr0) */
    enablePaging();
}


/*
 * get_pageTable_entry
 *   DESCRIPTION: returns entry in page table
 *   INPUTS: dir_idx: index in page directory; table_idx: index in page table
 *   OUTPUTS: entry if present, NULL if not present
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
pt_entry_t* get_pageTable_entry(uint32_t dir_idx, uint32_t table_idx) {
    pd_entry_t* page_dir;
	pd_entry_t dir_entry;
	pd_entry_smallPage_t smallPage;
	uint32_t table_addr;
	pt_entry_t* pt_entry;

    /* assert that indeces are within bounds of directory and table */
    if ((dir_idx < 0) || (dir_idx >= 1024)) {
        return NULL;
    }
    if ((table_idx < 0) || (table_idx >= 1024)) {
        return NULL;
    }

    /* get page dir ptr and index to get page table */
	page_dir = ret_dir_ptr();
	dir_entry = page_dir[dir_idx];

    /* assert that the page table exists */
	smallPage = dir_entry.smallPage;
    if (((smallPage.present) & 0x1) == 0) {
        return NULL;
    }

    /* get address of page table */
	table_addr = smallPage.page_table_base_addr;
	table_addr = table_addr << 12;
	pt_entry = (pt_entry_t*) (table_addr) + table_idx;
    /* assert the page exists */
    if (((pt_entry->present) & 0x1) == 0) {
        return NULL;
    }

    return pt_entry;
}


/*
 * get_bigPage
 *   DESCRIPTION: returns entry in page directory
 *   INPUTS: dir_idx: index in page directory
 *   OUTPUTS: entry if present, NULL if not present
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
pd_entry_bigPage_t* get_bigPage(uint32_t dir_idx) {
    pd_entry_t* page_dir;
	pd_entry_t dir_entry;
	pd_entry_bigPage_t bigPage;

    /* assert that indeces are within bounds of directory */
    if ((dir_idx < 0) || (dir_idx >= 1024)) {
        return NULL;
    }

    /* get ptr to page directory and index to get big page */
	page_dir = ret_dir_ptr();
	dir_entry = page_dir[dir_idx];

    /* assert that the page table exists */
	bigPage = dir_entry.bigPage;
    if (((bigPage.present) & 0x1) == 0) {
        return NULL;
    }

    return &page_dir[dir_idx].bigPage;
}
