#include "userprog/syscall.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "userprog/fd.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/mmu.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "intrinsic.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);
static struct fd_entry *find_fd_entry (int fd);
static bool is_valid_ptr (const void *ptr);
static bool is_valid_buffer (const void *buffer, int size);
static bool is_valid_string (const char *str);
static bool copy_user_string_to_page (const char *src, char *dst);
static void sys_exit (int status);

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

#define FILE_TYPE 3

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
is_valid_ptr (const void *ptr) {
	if (ptr == NULL || !is_user_vaddr (ptr))
		return false;
	return pml4_get_page (thread_current ()->pml4, ptr) != NULL;
}

static bool
is_valid_buffer (const void *buffer, int size) {
	if (size < 0)
		return false;
	if (size == 0)
		return true;
	if (buffer == NULL)
		return false;

	uint64_t start = (uint64_t) buffer;
	uint64_t end = start + size - 1;
	if (end < start)
		return false;

	for (uint64_t page = (uint64_t) pg_round_down ((void *) start);
			page <= end;
			page += PGSIZE) {
		if (!is_valid_ptr ((const void *) page))
			return false;
	}
	return true;
}

static bool
is_valid_string (const char *str) {
	for (;;) {
		if (!is_valid_ptr (str))
			return false;
		if (*str == '\0')
			return true;
		str++;
	}
}

static bool
copy_user_string_to_page (const char *src, char *dst) {
	for (size_t i = 0; i < PGSIZE; i++) {
		if (!is_valid_ptr (src + i))
			return false;
		dst[i] = src[i];
		if (dst[i] == '\0')
			return true;
	}
	return false;
}

static void
sys_exit (int status) {
	struct child_status *cs = thread_current ()->wait_status;
	if (cs != NULL)
		cs->exit_code = status;
	thread_exit ();
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f) {
	int syscall_num = f->R.rax;

	switch (syscall_num)
	{
		case SYS_HALT:
		{
			power_off ();
			break;
		}
		case SYS_FORK:
		{
			char *thread_name = (char *) f->R.rdi;
			if (!is_valid_string (thread_name))
				sys_exit (-1);
			f->R.rax = (tid_t) process_fork (thread_name, f);
			break;
		}
		case SYS_EXEC:
		{
			char *cmd_line = (char *) f->R.rdi;
			if (!is_valid_string (cmd_line))
				sys_exit (-1);
			char *cmd_line_copy = palloc_get_page (0);
			if (cmd_line_copy == NULL)
				sys_exit (-1);
			if (!copy_user_string_to_page (cmd_line, cmd_line_copy)) {
				palloc_free_page (cmd_line_copy);
				sys_exit (-1);
			}
			if (process_exec (cmd_line_copy) == -1)
				sys_exit (-1);
			break;
		}
		case SYS_WAIT:
		{
			tid_t tid = (tid_t) f->R.rdi;
			f->R.rax = process_wait (tid);
			break;
		}
		case SYS_EXIT:
		{
			sys_exit ((int) f->R.rdi);
			break;
		}
		case SYS_CREATE:
		{
			char *file_name = (char *) f->R.rdi;
			unsigned initial_size = f->R.rsi;
			if (!is_valid_string (file_name))
				sys_exit (-1);
			f->R.rax = filesys_create (file_name, initial_size);
			break;
		}
		case SYS_REMOVE:
		{
			char *file_name = (char *) f->R.rdi;
			if (!is_valid_string (file_name))
				sys_exit (-1);
			
			f->R.rax = filesys_remove(file_name);
			break;
		}
		case SYS_OPEN:
		{
			struct thread *cur = thread_current ();
			char *file_name = (char *) f->R.rdi;
			if (!is_valid_string (file_name))
				sys_exit (-1);

			struct file *file = filesys_open (file_name);
			if (file == NULL) {
				f->R.rax = -1;
				break;
			}

			struct fd_entry *entry = malloc (sizeof *entry);
			if (entry == NULL) {
				file_close (file);
				f->R.rax = -1;
				break;
			}

			int max_fd = 1;
			for (struct list_elem *e = list_begin (&cur->fd_table);
					e != list_end (&cur->fd_table);
					e = list_next (e)) {
				struct fd_entry *fd_entry =
					list_entry (e, struct fd_entry, file_elem);
				if (fd_entry->fd > max_fd)
					max_fd = fd_entry->fd;
			}

			entry->fd = max_fd + 1;
			entry->sfd = malloc (sizeof (struct shared_fd));
			entry->sfd->type = FILE_TYPE;
			entry->sfd->file = file;
			entry->sfd->shared_count = 1;
			list_push_back (&cur->fd_table, &entry->file_elem);
			f->R.rax = entry->fd;
			break;
		}
		case SYS_FILESIZE:
		{
			int fd = f->R.rdi;	
			struct fd_entry *entry = find_fd_entry(fd);
			if (entry == NULL) {
				f->R.rax = -1;
				break;
			}
			f->R.rax = file_length(entry->sfd->file);
			break;
		}
		case SYS_READ:
		{
			int fd = (int) f->R.rdi;
			char *buf = (char *) f->R.rsi;
			int size = (int) f->R.rdx;
			if (!is_valid_buffer (buf, size))
				sys_exit (-1);

			if (size == 0) {
				f->R.rax = 0;
				break;
			}

			if (fd == STDIN_FILENO) {
				for (int i = 0; i < size; i++)
					buf[i] = input_getc ();
				f->R.rax = size;
				break;
			}

			if (fd == STDOUT_FILENO) {
				f->R.rax = -1;
				break;
			}

			struct fd_entry *fd_entry = find_fd_entry (fd);
			if (fd_entry == NULL) {
				f->R.rax = -1;
				break;
			}

			f->R.rax = file_read (fd_entry->sfd->file, buf, size);
			break;
		}
		case SYS_WRITE:
		{
			int fd = (int) f->R.rdi;
			char *buf = (char *) f->R.rsi;
			int size = (int) f->R.rdx;
			if (buf == NULL || !is_valid_buffer (buf, size))
				sys_exit (-1);

			if (fd == STDOUT_FILENO) {
				putbuf (buf, size);
				f->R.rax = size;
				break;
			}

			struct fd_entry *entry = find_fd_entry (fd);
			if (entry == NULL) {
				f->R.rax = -1;
				break;
			}
			f->R.rax = file_write(entry->sfd->file, buf, size);
			break;
		}
		case SYS_SEEK:
		{
			int fd = (int) f->R.rdi;
			unsigned pos = (unsigned) f->R.rsi;
			if (pos < 0) {
				pos = 0;
			}
			struct fd_entry *fd_entry = find_fd_entry (fd);
			if (fd_entry == NULL) {
				break;
			}
			file_seek (fd_entry->sfd->file, pos);
			break;
		}
		case SYS_TELL:
		{
			int fd = (int) f->R.rdi;
			struct fd_entry *fd_entry = find_fd_entry (fd);
			if (fd_entry == NULL) {
				f->R.rax = -1;
				break;
			}
			f->R.rax = file_tell (fd_entry->sfd->file);
			break;
		}
		case SYS_CLOSE:
		{
			int fd = f->R.rdi;
			struct fd_entry *fd_entry = find_fd_entry (fd);
			if (fd_entry == NULL) {
				f->R.rax = -1;
				break;
			}

			list_remove (&fd_entry->file_elem);
			fd_entry->sfd->shared_count--;
			if (fd_entry->sfd->shared_count == 0)
			{
				file_close (fd_entry->sfd->file);
			}
			free (fd_entry);
			break;
		}
		case SYS_DUP2:
		{
			int fd_1 = f->R.rdi;
			int fd_2 = f->R.rsi;
			struct fd_entry *fd_entry_1 = find_fd_entry (fd_1);
			if (fd_entry_1 == NULL) {
				f->R.rax = -1;
				break;
			}
			struct fd_entry *fd_entry_2 = find_fd_entry (fd_2);
			if ( fd_entry_2 != NULL && fd_entry_1->sfd == fd_entry_2->sfd )
			{
				f->R.rax = fd_entry_2->fd;
				break;
			}
			if (fd_entry_2 == NULL) 
			{
				struct fd_entry *entry = malloc (sizeof *entry);
				struct thread *cur = thread_current();
				if (entry == NULL) {
					f->R.rax = -1;
					break;
				}
				entry->sfd = malloc ( sizeof (struct shared_fd));
				entry->fd = fd_2;
				entry->sfd = fd_entry_1->sfd;
				entry->sfd->shared_count++;
				list_push_back (&cur->fd_table, &entry->file_elem);
				f->R.rax = entry->fd;	

			}
			else
			{
				fd_entry_2->sfd->shared_count--;
				if (fd_entry_2->sfd->shared_count == 0)
				{
					file_close (fd_entry_2->sfd->file);
				}
				fd_entry_2->sfd = fd_entry_1->sfd;
				fd_entry_2->sfd->shared_count++;
				f->R.rax = fd_entry_2->fd;
			}
			
			break;

		}
		default:
			sys_exit (-1);
			break;
	}
}

static struct fd_entry *
find_fd_entry (int fd) {
	struct thread *cur = thread_current ();
	for (struct list_elem *e = list_begin (&cur->fd_table);
			e != list_end (&cur->fd_table);
			e = list_next (e)) {
		struct fd_entry *fd_entry = list_entry (e, struct fd_entry, file_elem);
		if (fd_entry->fd == fd)
			return fd_entry;
	}

	return NULL;
}
