#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "threads/init.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

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
static bool
is_valid_ptr(char *buf)
{
	if ( !is_user_vaddr(buf) || pml4_get_page (thread_current ()->pml4, buf ) == NULL)
		return false;
	return true;
}
static bool
is_valid_buffer(char *buf, int size)
{
	for(int64_t i = pg_round_down (buf) ; i < buf + size - 1 ; i += PGSIZE)
	{
		if( !is_user_vaddr(i) || pml4_get_page (thread_current ()->pml4, i ) == NULL)
		{
			return false;
		}
	}

	return true;
}
static bool
is_valid_string(char *buf)
{
	for( int i=0 ; ; i++)
	{
		if ( !is_valid_ptr( buf + i ) )
		{
			return false;
		}
		if( buf[i] == '\0'){
			break;
		}
	}
	return true;
}
/* The main system call interface */
void
syscall_handler (struct intr_frame *f) {
	int syscall_num = f->R.rax;

	switch (syscall_num)
	{
		case SYS_WRITE:
		{
			int fd = (int) f->R.rdi;
			char *buf = (char*) f->R.rsi;
			int size = (int) f->R.rdx;
			if( buf == NULL || !is_valid_buffer( buf, size ) )
			{
				// TODO: exit(-1))
				return ;
			}

			if (fd == STDOUT_FILENO) {
				putbuf(buf, size);
				f->R.rax = size;	// write()의 반환값으로 출력한 바이트 수를 돌려준다.
			}
			break;
		}

		case SYS_READ:
		{
			int fd = (int) f->R.rdi;
			char *buf = (char*) f->R.rsi;
			int size = (int) f->R.rdx;
			if( buf == NULL || !is_valid_buffer( buf, size ) )
			{
				// TODO: exit(-1))
				return ;
			}

			break;
		}

		case SYS_FORK:
		{
			char *thread_name = (char*) f->R.rdi;
			if( !is_valid_string( thread_name ) )
			{
				// TODO: exit(-1))
				return ;
			}
			//TODO: SYS_FORK
			break;
		}

		case SYS_EXEC:
		{
			char *cmd_line = (char*) f->R.rdi;
			if( !is_valid_string( cmd_line ) )
			{
				// TODO: exit(-1))
				return ;
			}

			break;
		}

		case SYS_CREATE :
		{
			char *file_name = (char*) f->R.rdi;
			if( !is_valid_string( file_name ) )
			{
				// TODO: exit(-1))
				return ;
			}

			break;
		}

		case SYS_REMOVE :
		{
			char *file_name = (char*) f->R.rdi;
			if( !is_valid_string( file_name ) )
			{
				// TODO: exit(-1))
				return ;
			}

			break;
		}

		case SYS_OPEN :
		{
			char *file_name = (char*) f->R.rdi;
			if( !is_valid_string( file_name ) )
			{
				// TODO: exit(-1))
				return ;
			}

			break;
		}

		case SYS_EXIT:
		{ 
			#ifdef USERPROG
			thread_current()->exit_status = (int) f->R.rdi;
			thread_exit();
			#endif
			break;
		}
        case SYS_HALT:
		{
			printf("Syetem Halted\n");
			power_off();
			break;
		}
		default:
		    thread_current()->exit_status = -1;
			thread_exit(); //알 수 없는 syscall이 들어오면 비정상 종료 상태(-1)를 저장하고 종료한다
            break;
	}
}
