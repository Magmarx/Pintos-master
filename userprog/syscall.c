#include "userprog/syscall.h"
#include <user/syscall.h>

#include <stdio.h>
#include <syscall-nr.h>

#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

/********* Global Vars **********/
#define MAX_ARGS 3

bool FILE_LOCK_INIT = false;

/********* Function Declaration ********/
//args
void get_args (struct intr_frame *f, int *arg, int num_of_args);

//pointers
void validate_ptr (const void* vaddr);

//syscall
static void syscall_handler (struct intr_frame *);

/************ Syscall **************/

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!FILE_LOCK_INIT) {
    lock_init(&file_system_lock);
    FILE_LOCK_INIT = true;
  }
  
  int arg[MAX_ARGS];
  int esp = getpage_ptr((const void *) f->esp);

  switch (* (int *)esp) {
    case SYS_EXIT:
      // First we fill all the args with the amound it needs
      get_args(f, &arg[0], 1);
      syscall_exit(arg[0]);
      break;
    case SYS_WAIT:
      // fill arg with the amount of arguments needed
      get_args(f, &arg[0], 1);
      f->eax = syscall_wait(arg[0]);
      break;
    default:
      break;
  }
}


/*
  This function checks if the exiting thread is the current thread
  If true we update the parent status
*/
void
syscall_exit (int status)
{
  struct thread *cur = thread_current();

  if (is_thread_alive(cur->parent) && cur->cp) {
    if (status < 0) {
      status = -1;
    }
    cur->cp->status = status;
  }

  printf("%s: exit(%d)\n", cur->name, status);
  thread_exit();
}

/*
  Here we make the syscall wait
*/
int
syscall_wait(pid_t pid)
{
  return process_wait(pid);
}

/************ Pages **************/

/*
  This function gets the pointer to the active page from the thread
*/
int
getpage_ptr(const void *vaddr)
{
  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr) {
    syscall_exit(ERROR);
  }
  return (int)ptr;
}

/************ args **************/

void
get_args (struct intr_frame *f, int *args, int num_of_args)
{
  int i;
  int *ptr;

  for (i = 0; i < num_of_args; i++) {
    ptr = (int *) f->esp + i + 1;
    validate_ptr((const void *) ptr);
    args[i] = *ptr;
  }
}

/************ pointers ***********/

/*
  This function validates the pointer 
*/
void
validate_ptr (const void *vaddr)
{
    if (vaddr < USER_VADDR_BOTTOM || !is_user_vaddr(vaddr)) {
      // Err: out of bound memory access
      syscall_exit(ERROR);
    }
}

/************ Files **************/

/* close the desired file descriptor */
void
process_close_file (int file_descriptor)
{
  struct thread *t = thread_current();
  struct list_elem *next;
  struct list_elem *e = list_begin(&t->file_list);
  
  for (;e != list_end(&t->file_list); e = next) {

    next = list_next(e);
    struct process_file *process_file_ptr = list_entry (e, struct process_file, elem);
    
    if (file_descriptor == process_file_ptr->fd || file_descriptor == CLOSE_ALL_FD) {

      file_close(process_file_ptr->file);
      list_remove(&process_file_ptr->elem);
      free(process_file_ptr);

      if (file_descriptor != CLOSE_ALL_FD) {
        return;
      }

    }
  }
}

/************ Child Process **************/

/* Remove all the child processes in a thread */
void remove_all_child_processes (void) 
{
  struct thread *t = thread_current();
  struct list_elem *next;
  struct list_elem *e = list_begin(&t->child_list);
  
  // For all the child processes in the thread we will remove it
  for (;e != list_end(&t->child_list); e = next) {
    next = list_next(e);
    struct child_process *cp = list_entry(e, struct child_process, elem);
    list_remove(&cp->elem);
    free(cp);
  }
}

/* remove a specific child process */
void
remove_child_process (struct child_process *cp)
{
  list_remove(&cp->elem);
  free(cp);
}

/* find a child process based on pid */
struct child_process* find_child_process(int pid)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct list_elem *next;
  
  for (e = list_begin(&t->child_list); e != list_end(&t->child_list); e = next)
  {
    next = list_next(e);
    struct child_process *cp = list_entry(e, struct child_process, elem);
    if (pid == cp->pid)
    {
      return cp;
    }
  }
  return NULL;
}