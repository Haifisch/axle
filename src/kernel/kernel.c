#include <kernel/kernel.h>
#include <user/shell/shell.h>
#include <kernel/drivers/rtc/clock.h>
#include <std/common.h>
#include <kernel/util/paging/descriptor_tables.h>
#include <kernel/drivers/pit/pit.h>
#include <kernel/util/paging/paging.h>
#include <stdarg.h>
#include <gfx/lib/gfx.h>
#include <kernel/drivers/vesa/vesa.h>
#include <std/kheap.h>
#include <tests/test.h>
#include <user/xserv/xserv.h>
#include "multiboot.h"
#include <gfx/font/font.h>
#include <kernel/util/multitasking/task.h>
#include <gfx/lib/view.h>
#include <kernel/util/syscall/syscall.h>
#include <kernel/util/mutex/mutex.h>
#include <std/printf.h>

void print_os_name(void) {
	printf("\e[10;[\e[11;AXLE OS v\e[12;0.4.0\e[10;]\n");
}

void shell_loop(void) {
	int exit_status = 0;
	while (!exit_status) {
		exit_status = shell();
	}

	//give them a chance to recover
	for (int i = 5; i > 0; i--) {
		terminal_clear();
		printf_info("Shutting down in %d. Press any key to cancel", i);
		sleep(1000);
		if (haskey()) {
			//clear buffer
			while (haskey()) {
				getchar();
			}
			//restart shell loop
			terminal_clear();
			shell_loop();
			break;
		}
	}
	
	//we're dead
	terminal_clear();
} 

void kernel_begin_critical(void) {
	//disable interrupts while critical code executes
	asm ("cli");
}

void kernel_end_critical(void) {
	//reenable interrupts now that a critical section is complete
	asm ("sti");
}

void info_panel_refresh(void) {
	/*
	term_cursor pos = terminal_getcursor();

	//set cursor near top right, leaving space to write
	term_cursor curs = (term_cursor){65, 0};
	terminal_setcursor(curs);

	printf("PIT: %d", tick_count());
	//using \n would move cursor x = 0
	//instead, manually set to next row
	curs.y += 1;
	terminal_setcursor(curs);
	printf("RTC: %d", time());

	//now that we're done, put the cursor back
	terminal_setcursor(pos);
	*/
}

void info_panel_install(void) {
	printf_info("Installing text-mode info panel...");
	timer_callback info_callback = add_callback(info_panel_refresh, 1, 1, NULL);
}

extern uint32_t placement_address;
uint32_t initial_esp;

void kernel_main(multiboot* mboot_ptr, uint32_t initial_stack) {
	initial_esp = initial_stack;

	//initialize terminal interface
	terminal_initialize();

	//introductory message
	print_os_name();
	
	//run color test
	test_colors();

	printf_info("Available memory:");
	printf("%d -> %dMB\n", mboot_ptr->mem_upper, (mboot_ptr->mem_upper/1024));
	
	gdt_install();
	idt_install();
	//test_interrupts();

	pit_install(1000);

	paging_install();

	syscall_install();

	tasking_install();
	
	kb_install();

	test_heap();

	//set up info panel
	info_panel_install();

	test_printf();

	test_time_unique();

	test_malloc();

	test_crypto();

	shell_init();
	shell_loop();
	
	while (1) {}
}


