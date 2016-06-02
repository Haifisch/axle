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

void print_os_name() {
	terminal_settextcolor(COLOR_GREEN);
	printf("[");
	terminal_settextcolor(COLOR_LIGHT_CYAN);
	printf("AXLE OS v");
	terminal_settextcolor(COLOR_LIGHT_RED);
	printf("0.3.0");
	terminal_settextcolor(COLOR_GREEN);
	printf("]\n");
}

void shell_loop() {
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

void kernel_begin_critical() {
	//disable interrupts while critical code executes
	asm ("cli");
}

void kernel_end_critical() {
	//reenable interrupts now that a critical section is complete
	asm ("sti");
}

void info_panel_refresh() {
	cursor pos = get_cursor();

	//set cursor near top left, leaving space to write
	cursor curs;
	curs.x = 65;
	curs.y = 0;
	set_cursor(curs);

	printf("PIT: %d", tick_count());
	//using \n would move cursor x = 0
	//instead, manually set to next row
	curs.y += 1;
	set_cursor(curs);
	printf("RTC: %d", time());

	//now that we're done, put the cursor back
	set_cursor(pos);
}

void info_panel_install() {
	printf_info("Installing text-mode info panel...");
	timer_callback info_callback = add_callback(info_panel_refresh, 1, 1, NULL);
}

extern uint32_t placement_address;
uint32_t initial_esp;

#if defined(__cplusplus)
extern "C" //use C linkage for kernel_main
#endif
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

	//force_page_fault();
	//force_hardware_irq();	
/*
	static int ret = 6;
	static int a_count = 0;
	static int b_count = 0;
	for (int i = PRIO_LOW; i <= PRIO_HIGH; i++) {
		ret = fork(i);

		printf_info("using prio %d", i);

		a_count = 0;
		b_count = 0;
		while (1) {
			if (b_count > 50) {
				if (!ret) {
					printf_info("Process [%d] got %d/%d quantums", getpid(), b_count, (a_count + b_count));
					break;
				}
				else continue;
			}
			else {
				if (!ret) b_count++;
				a_count++;
				//yield();
			}
		}
	}
*/
/*
	for (int i = PRIO_LOW; i < PRIO_MED; i++) {
		switch (i) {
			case PRIO_LOW:
			default:
				printf_info("PRIO_LOW");
				break;
			case PRIO_MED:
				printf_info("PRIO_MED");
				break;
			case PRIO_HIGH:
				printf_info("PRIO_HIGH");
				break;
		}

		int ret = fork(i);
		static int a_count = 0;
		static int b_count = 0;
		while (1) {
			if (a_count > 1000 || b_count > 1000) break;
			if (ret) a_count++;
			else b_count++;
			yield();
			//if (!ret) printf_info("child");
			//else printf_info("parent");
		}

		if (!ret) printf_info("a_count: %d b_count %d", a_count, b_count);
	}
*/
	while (1) {}

	shell_init();
	shell_loop();

	while (1) {}
}


