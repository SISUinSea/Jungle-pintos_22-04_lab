#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

struct fd_entry {
	int fd;
	struct file *file;
	struct list_elem file_elem;
};

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	int syscall_num = f->R.rax;

	switch (syscall_num)
	{
		case SYS_EXIT:
		{
			char *name = thread_current ()->name;
			int status_code = (int) f->R.rdi;
			printf("%s: exit(%d)\n", name, status_code);
			thread_exit ();
			break;
		}
		case SYS_OPEN:
		{
			
		}
		case SYS_FILESIZE:
		{

		}
		case SYS_READ:
		{

		}
		case SYS_WRITE:
		{
			int fd = (int) f->R.rdi;
			char *buf = (char*) f->R.rsi;
			int size = (int) f->R.rdx;

			if (fd == STDOUT_FILENO) {
				putbuf(buf, size);	
				f->R.rax = size;
				break;
			}
			f->R.rax = -1;
			break;
		}
		default:
			break;
	}
}
