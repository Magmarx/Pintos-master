#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

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
  printf ("system call!\n");
  thread_exit ();
}


/*
  This function checks if the exiting thread is the current thread
  If true we update the parent status
*/
void
syscall_exit (int status)
{
  struct thread *cur = thread_current();
  if (is_thread_alive(cur->parent) && cur->cp)
  {
    if (status < 0)
    {
      status = -1;
    }
    cur->cp->status = status;
  }
  printf("Exiting thread %s: exit(%d)\n", cur->name, status);
  thread_exit();
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