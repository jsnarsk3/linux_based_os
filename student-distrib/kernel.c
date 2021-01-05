/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "paging_c.h"
#include "exceptions.h"
#include "filesys.h"
#include "rtc_driver.h"
#include "key_driver.h"

#define RUN_TESTS
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))

// Macros for IDT
#define SYS_CALL_NUM 0x80
#define TASK_GATE 0x05
#define INT_GATE  0x0E
#define TRAP_GATE 0x0F
#define size_mask 0x08
#define res_1_mask 0x04
#define res_2_mask 0x02
#define res_3_mask 0x01

// Exception Assembly Linkage Function
void* idt_functions[] = {DIVIDE_ERROR, (int *)1, NMI_INTERRUPT, BREAKPOINT, OVERFLOW, BOUND_RANGE,
						INVALID_OPCODE, DEVICE_NOT_AVAILABLE, DOUBLE_FAULT, (int *)1, INVALID_TSS,
						SEGMENT_MISSING, STACK_SEG_FAULT, GENERAL_PROT, PAGE_FAULT, (int * )1,
						FLOAT_ERROR, ALIGN_CHECK, MACHINE_CHECK, FLOAT_EXCEP};

// Protection Level for each exception
int idt_protections[] = {0,0,0,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Gate Types for each exception
int idt_gates[] = {TRAP_GATE, TRAP_GATE, INT_GATE, INT_GATE, TRAP_GATE, TRAP_GATE, TRAP_GATE, TRAP_GATE, TASK_GATE,
				   TRAP_GATE, TRAP_GATE, TRAP_GATE, TRAP_GATE, TRAP_GATE, INT_GATE, TRAP_GATE, TRAP_GATE, TRAP_GATE,
				   TRAP_GATE, TRAP_GATE};

/* void set_idt_entry();
 * Inputs: unsigned int vector_number : vector number in the idt to write to
 *         void* in_function() : function pointer to a handler to place in the idt, optional
 * Return Value: none
 * Function: Sets the correct bits in the idt_struct and links the inputted handler for the given vector number
*/
void set_idt_entry(unsigned int vector_number, void* in_function()){
		int size, res1, res2, res3;
		int cur_gate;
		// Passed in a non-exception function
		if(in_function != NULL){
			SET_IDT_ENTRY(idt[vector_number], in_function);
			idt[vector_number].dpl = 0;
			idt[vector_number].present = 1;
			idt[vector_number].seg_selector = KERNEL_CS;
			idt[vector_number].reserved0 = 0;
			idt[vector_number].size = 1;
			idt[vector_number].reserved1 = 1;
			idt[vector_number].reserved2 = 1;
			idt[vector_number].reserved3 = 0;
			idt[vector_number].reserved4 = 0;
		}
		else{
			// Exception into the IDT
			if(vector_number<20 && vector_number != 1 && vector_number != 9 && vector_number!= 15){
				cur_gate = idt_gates[vector_number];
				size = (cur_gate & size_mask )>> 3;
				res1 = (cur_gate & res_1_mask)>> 2;
				res2 = (cur_gate & res_2_mask)>> 1;
				res3 = (cur_gate & res_3_mask)>> 0;
				// Task Gate
				if(idt_gates[vector_number]==TASK_GATE){
					SET_IDT_ENTRY(idt[vector_number], 1);
					idt[vector_number].seg_selector = KERNEL_TSS;
					idt[vector_number].present = 1;
					idt[vector_number].dpl = 0;
				}
				// Other Gate
				else{
					SET_IDT_ENTRY(idt[vector_number], idt_functions[vector_number]);
					idt[vector_number].dpl = idt_protections[vector_number];
					idt[vector_number].present = 1;
					idt[vector_number].seg_selector = KERNEL_CS;
				}
				// Set the reserved bits according to gate type
				idt[vector_number].reserved0 = 0;
				idt[vector_number].size = size;
				idt[vector_number].reserved1 = res1;
				idt[vector_number].reserved2 = res2;
				idt[vector_number].reserved3 = res3;
				idt[vector_number].reserved4 = 0;
			}
			// Initialize and Fill the System Call Handler in idt
			else if(vector_number == SYS_CALL_NUM){
				cur_gate = TRAP_GATE;
				size = (cur_gate & size_mask )>> 3;
				res1 = (cur_gate & res_1_mask)>> 2;
				res2 = (cur_gate & res_2_mask)>> 1;
				res3 = (cur_gate & res_3_mask)>> 0;
				SET_IDT_ENTRY(idt[vector_number], SYS_CALL_HANDLER);
				idt[vector_number].seg_selector = KERNEL_CS;
				idt[vector_number].dpl = 3;
				idt[vector_number].present = 1;
				idt[vector_number].reserved0 = 0;
				idt[vector_number].size = size;
				idt[vector_number].reserved1 = res1;
				idt[vector_number].reserved2 = res2;
				idt[vector_number].reserved3 = res3;
				idt[vector_number].reserved4 = 0;
			}
			// Fill an empty spot in the idt with a non-null value
			else if(idt[vector_number].present == 0){
				SET_IDT_ENTRY(idt[vector_number], 1);
				idt[vector_number].dpl = 0;
				idt[vector_number].present = 0;
				idt[vector_number].seg_selector = KERNEL_CS;
				idt[vector_number].reserved0 = 0;
				idt[vector_number].size = 1;
				idt[vector_number].reserved1 = 1;
				idt[vector_number].reserved2 = 1;
				idt[vector_number].reserved3 = 0;
				idt[vector_number].reserved4 = 0;
			}
		}
}
/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;
	uint32_t filesys_addr;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
		/* filesys_img addr is the 0th module */
		filesys_addr = mod->mod_start;
        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }
	// Disable Interrupts
	cli();

	// Clear the screen
	clear();

	unsigned int i = 0;
	// Load the idt register with location of idt
	lidt(idt_desc_ptr);

    /* Init the PIC */
    i8259_init();

	// Mask all the interrupts on the PIC
	for(i = 0; i < 16; i++){
		disable_irq(i);
	}

	// Enable the RTC
	rtc_init();
	set_idt_entry(PIC_SLAVE_IDT, (void *)RTC_HANDLER);
	set_idt_entry(PIC_MASTER_IDT+1, (void *)KEY_HANDLER);

	// Initialize and Fill the IDT
	// Fill the idt table with exceptions
	for(i = 0; i < 256; i++){
		set_idt_entry(i, NULL);
	}
    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    printf("Enabling Interrupts\n");
    sti();

	init_paging();

	init_filesys((uint32_t*) filesys_addr);
#ifdef RUN_TESTS
    /* Run tests */
    clear();
#endif
    /* Execute the first program ("shell") ... */
    /* Spin (nicely, so we don't chew up cycles) */
	set_screen(0,0);
	// Execute
	while (1) {
		asm("movl %0, %%ebx\n"
			"movl %1, %%eax"
			:
			: "r"("shell"), "r"(2)
			: "eax", "ebx"
		);
		asm("int $0x80");
	}
	asm volatile (".1:hlt; jmp .1;");
}
