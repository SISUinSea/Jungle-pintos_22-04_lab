#ifndef USERPROG_FD_H
#define USERPROG_FD_H

#include <list.h>

struct file;

struct fd_entry {
	int fd;
	struct file *file;
	struct list_elem file_elem;
};

#endif /* userprog/fd.h */