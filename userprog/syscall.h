#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"
#include "threads/thread.h"
void syscall_init (void);

#define ERROR -1
#define NOT_LOADED 0
#define LOADED 1
#define LOAD_FAIL 2
#define CLOSE_ALL_FD -1
#define USER_VADDR_BOTTOM ((void *) 0x08048000)

struct process_file {
    struct file *file;
    int fd;
    struct list_elem elem;
};

struct child_process {
  int pid;
  int load_status;
  int wait;
  int exit;
  int status;
  struct semaphore load_sema;
  struct semaphore exit_sema;
  struct list_elem elem;
};

struct lock file_system_lock;

/******** syscall ***********/
void syscall_exit (int status);

/******** Child process **********/
struct child_process* find_child_process (int pid);
void remove_child_process (struct child_process *child);
void remove_all_child_processes (void);

/******** Files **********/
void process_close_file (int file_descriptor);
struct file* get_file(int filedes);

/******** Pages *********/
int getpage_ptr (const void *vaddr);

#endif /* userprog/syscall.h */
