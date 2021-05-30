#include "userprog/syscall.h"
#include <user/syscall.h>

#include <stdio.h>
#include <syscall-nr.h>

#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "filesys/file.h"

#include "userprog/libreriasSyscall.h"
#include "userprog/corrimientosAritmeticos.h"

bool FILE_LOCK_INIT = false;

//inicializamos para que no tenga contenido basura
int i=0;
const int *ptr=0;

unsigned contadorVerificadorBuffer = 0;

struct lock memoriaDeLock;

char* local_buffer = NULL;



void syscall_init (void) 
{
  lock_init(&memoriaDeLock);
  intr_register_int (DIRECCION_0X30,TOKEN3, INTR_ON, syscall_handler, SYSCALL);
}

static void syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!FILE_LOCK_INIT) {
    lock_init(&file_system_lock);
    FILE_LOCK_INIT = true;
  }
  
  int arg[MAX_ARGS];
  int esp = getpage_ptr((const void *) f->esp);

  switch (* (int *)esp) {
    case SYS_HALT:
      /*Detenemos pintos "Apagando la computadora"
      Segun Stanford: terminates pintos by calling 
      shutdown_power_off, declarado en threads/init.h
      pocs veces se debe usar
      */
      shutdown_power_off();
      break;

    case SYS_CREATE:
      // se va llenando el arreglo con la cantidad de argumentos que son necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 2);
      
      //valida si la linea del comando es valido
      validate_str((const void *)arg[NUMERO0]);
      
      // obtenemos el puntero de la pagina
      arg[NUMERO0] = getpage_ptr((const void *) arg[NUMERO0]);
      
      /* creamos el syscall le mandamos el nombre del archivo y el tamanio sin signo */
      f->eax = syscall_create((const char *)arg[NUMERO0], (unsigned)arg[NUMERO1]);  // create this file
      break;

    case SYS_REMOVE:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 1);
      
      /* comprobamos si la linea de comando es correcta  */
      validate_str((const void*)arg[NUMERO0]);
      
      //vamos a obtener el puntero de pagina 
      arg[NUMERO0] = getpage_ptr((const void *) arg[NUMERO0]);
      
      /* syscall_remove(const char* file_name) */
      f->eax = syscall_remove((const char *)arg[NUMERO0]);  // elimina este archivo 
      break;

    case SYS_OPEN:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 1);
      
      /* vamos a comprobar si la linea de comando es valida para no abrir basura que 
      pueda probocar bloqueos 
       */
       validate_str((const void*)arg[NUMERO0]);
     
     // obtenemos el puntero de pagina 
      arg[NUMERO0] = getpage_ptr((const void *)arg[NUMERO0]);
      
      /* creamos syscall_open(int filedes) */
      f->eax = syscall_open((const char *)arg[NUMERO0]);  // abre este  archivo 
      break;

    case SYS_FILESIZE:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 1);
      
      /* creamos syscall_filesize (const char *file_name) le enviamos el nombre del archcivo */
      f->eax = syscall_filesize(arg[NUMERO0]);  // obtenemos el tamanio del archivo
      break;
    
    case SYS_WRITE:
      
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 3);
      
      /* verificamos si el buffer es valido 
       * no queremos tener un buffer que esta fuera de nuestra memoria virtual
       */
       validarBuffer((const void*)arg[NUMERO1], (unsigned)arg[2]);
       
     // obtenemos el puntero de pagina 
      arg[NUMERO1] = getpage_ptr((const void *)arg[NUMERO1]); 
      
      /* creamos syscall_write (int filedes, const void * buffer, unsigned bytes)*/
      f->eax = syscall_write(arg[NUMERO0], (const void *) arg[NUMERO1], (unsigned) arg[NUMERO2]);
      break;
    
    case SYS_TELL:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 1);
      /*  creamos syscall_tell(int filedes) */
      f->eax = syscall_tell(arg[NUMERO0]);
      break;
    
  

    case SYS_EXIT:
      // First we fill all the args with the amound it needs
      obtenerArgumentos(f, &arg[NUMERO0], 1);
      syscall_exit(arg[NUMERO0]);
      break;

    case SYS_EXEC:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 1);
      
      // verifica si la linea de comando es valida
      validate_str((const void*)arg[NUMERO0]);
      
      // obtenemos el puntero de la pagina 
      arg[NUMERO0] = getpage_ptr((const void *)arg[NUMERO0]);
      /* creamos  syscall_exec(const char* cmdline) */
      f->eax = syscall_exec((const char*)arg[NUMERO0]); // Ejecuta la linea de comando 

      break;

    case SYS_READ:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 3);
      
       /* verificamos si el buffer es valido 
       * no queremos tener un buffer que esta fuera de nuestra memoria virtual
       */
       validarBuffer((const void*)arg[NUMERO1], (unsigned)arg[2]);
      // obtenemos el puntero de la pagina 
      arg[NUMERO1] = getpage_ptr((const void *)arg[NUMERO1]); 
      
      /* creamos  syscall_write (int filedes, const void * buffer, unsigned bytes)*/
      f->eax = syscall_read(arg[NUMERO0], (void *) arg[NUMERO1], (unsigned) arg[2]);
      break;

    case SYS_SEEK:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[NUMERO0], 2);
      /* creamos  syscall_seek(int filedes, unsigned new_position) */
      syscall_seek(arg[NUMERO0], (unsigned)arg[NUMERO1]);
      break;

    case SYS_WAIT:
      // fill arg with the amount of arguments needed
      //obtenerArgumentos(f, &arg[NUMERO0], 1);
      // process.c
      f->eax = process_wait(arg[NUMERO0]);
      break;

    case SYS_CLOSE:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos (f, &arg[NUMERO0], 1);
      /* creamos syscall_close(int filedes) */
      syscall_close(arg[NUMERO0]);
      break;
  
    default:
      break;
  }
}

/*
Segun Stanford, se cambia process wait a un bucle
infinit. Esto hace que se apague antes que cualquier proceso

Si pid, esta vivo, espera
*/



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
obtenerArgumentos (struct intr_frame *f, int *args, int num_of_args)
{
  i = 0;
  while(i < num_of_args){
    i++;
    ptr = (int *) f->esp + i + 1;
    validarPunteros(ptr);
    args[i] = *ptr;
  }
}

/************ Buffer **************/
/* función para comprobar si el búfer es válido */
void
validarBuffer(const void* buf, unsigned byte_size)
{
  local_buffer = (char *)buf;
  for (; contadorVerificadorBuffer < byte_size; contadorVerificadorBuffer++)
  {
    validarPunteros((const void*)local_buffer);
    local_buffer++;
  }
}


/************ pointers ***********/

/*
  This function validates the pointer 
*/
void
validarPunteros (const void *vaddr)
{
    if (vaddr < USER_VADDR_BOTTOM || !is_user_vaddr(vaddr)) {
      // Err: out of bound memory access
      syscall_exit(ERROR);
    }
}


pid_t syscall_exec(const char* cmdline)
{
    pid_t pid = process_execute(cmdline);
    struct child_process *child_process_ptr = find_child_process(pid);
    if (!child_process_ptr)
    {
      return ERROR;
    }
    /* comprobar si el proceso está cargado */
    if (child_process_ptr->load_status == NOT_LOADED)
    {
      sema_down(&child_process_ptr->load_sema);
    }
    /* comprobar si el proceso no se pudo cargar */
    if (child_process_ptr->load_status == LOAD_FAIL)
    {
      remove_child_process(child_process_ptr);
      return ERROR;
    }
    return pid;
}

/************ String ***********/

/* función para comprobar que la cadena es válida */
void
validate_str (const void* str)
{
    for (; * (char *) getpage_ptr(str) != 0; str = (char *) str + 1);
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

int obtenerLongitud(int longitud, int condicion){
  if (longitud<=condicion)
  {
    /* code */
    return longitud;
  }
}

int
syscall_read(int filedes, void *buffer, unsigned length)
{
  obtenerLongitud(length,NUMERO0);  
  if (filedes == STD_INPUT)
  {
    unsigned i = 0;
    uint8_t *local_buf = (uint8_t *) buffer;
    for (;i < length; i++)
    {
      //recuperar la tecla presionando desde el buffer de entrada 
      local_buf[i] = input_getc(); // from input.h
    }
    return length;
  }
  
  /* read from file */
  lock_acquire(&file_system_lock);
  struct file *file_ptr = get_file(filedes);
  if (!file_ptr)
  {
    lock_release(&file_system_lock);
    return ERROR;
  }
  int bytes_read = file_read(file_ptr, buffer, length); // from file.h
  lock_release (&file_system_lock);
  return bytes_read;
}

/*obtener un archivo que coincida con el descriptor de archivo */
struct file*
get_file (int filedes)
{
  struct thread *t = thread_current();
  struct list_elem* next;
  struct list_elem* e = list_begin(&t->file_list);
  
  for (; e != list_end(&t->file_list); e = next)
  {
    next = list_next(e);
    struct process_file *process_file_ptr = list_entry(e, struct process_file, elem);
    if (filedes == process_file_ptr->fd)
    {
      return process_file_ptr->file;
    }
  }
  return NULL; // renturn null cuando no se encuentre nada 
}

/* syscall_seek */
void
syscall_seek (int filedes, unsigned new_position)
{
  lock_acquire(&file_system_lock);
  struct file *file_ptr = get_file(filedes);
  if (!file_ptr)
  {
    lock_release(&file_system_lock);
    return;
  }
  file_seek(file_ptr, new_position);
  lock_release(&file_system_lock);
}

/* syscall_close */
void
syscall_close(int filedes)
{
  lock_acquire(&file_system_lock);
  process_close_file(filedes);
  lock_release(&file_system_lock);
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

/************ syscall_ **************/

/*  creamos syscall_create */
bool syscall_create(const char* file_name, unsigned starting_size)
{
  lock_acquire(&file_system_lock);
  bool successful = filesys_create(file_name, starting_size); // from filesys.h
  lock_release(&file_system_lock);
  return successful;
}

/*  creamos syscall_remove */
bool syscall_remove(const char* file_name)
{
  lock_acquire(&file_system_lock);
  bool successful = filesys_remove(file_name); // from filesys.h
  lock_release(&file_system_lock);
  return successful;
}

/* cramos syscall_open */
int syscall_open(const char *file_name)
{
  lock_acquire(&file_system_lock);
  struct file *file_ptr = filesys_open(file_name); // from filesys.h
  if (!file_ptr)
  {
    lock_release(&file_system_lock);
    return ERROR;
  }
  int filedes = agregarArchivos(file_ptr);
  lock_release(&file_system_lock);
  return filedes;
}

/*
agregar archivo a la lista de archivos y devolver el descriptor de archivo del archivo agregado*/
int agregarArchivos (struct file *file_name)
{
  struct process_file *process_file_ptr = malloc(sizeof(struct process_file));
  if (!process_file_ptr)
  {
    return ERROR;
  }
  process_file_ptr->file = file_name;
  process_file_ptr->fd = thread_current()->fd;
  thread_current()->fd++;
  list_push_back(&thread_current()->file_list, &process_file_ptr->elem);
  return process_file_ptr->fd;
  
}

/* creamos  syscall_filesize */
int syscall_filesize(int filedes)
{
  lock_acquire(&file_system_lock);
  struct file *file_ptr = get_file(filedes);
  if (!file_ptr)
  {
    lock_release(&file_system_lock);
    return ERROR;
  }
  int filesize = file_length(file_ptr); // from file.h
  lock_release(&file_system_lock);
  return filesize;
}

/* creamos syscall_write */
int  syscall_write (int filedes, const void * buffer, unsigned byte_size)
{
    if (byte_size <= 0)
    {
      return byte_size;
    }
    if (filedes == STD_OUTPUT)
    {
      putbuf (buffer, byte_size); // from stdio.h
      return byte_size;
    }
    
    // se empiez a escribir en el archivo 
    lock_acquire(&file_system_lock);
    struct file *file_ptr = get_file(filedes);
    if (!file_ptr)
    {
      lock_release(&file_system_lock);
      return ERROR;
    }
    int bytes_written = file_write(file_ptr, buffer, byte_size); // file.h
    lock_release (&file_system_lock);
    return bytes_written;
}

/* syscall_tell */
unsigned
syscall_tell(int filedes)
{
  lock_acquire(&file_system_lock);
  struct file *file_ptr = get_file(filedes);
  if (!file_ptr)
  {
    lock_release(&file_system_lock);
    return ERROR;
  }
  off_t offset = file_tell(file_ptr); //from file.h
  return offset;
}