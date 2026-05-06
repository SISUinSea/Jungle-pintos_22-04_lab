#ifndef USERPROG_FD_H
#define USERPROG_FD_H

#include <list.h>

struct file;

struct fd_entry {
	int fd;
	struct shared_fd *sfd;
	struct list_elem file_elem;
};
struct shared_fd {
	int type;
	int shared_count;
	struct file *file;
};

#endif /* userprog/fd.h */