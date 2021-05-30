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
#include "corrimientosAritmeticos.h"
#include "libreriasSyscall.h"


bool FILE_LOCK_INIT = false;

int arg[MAX_ARGS];

void syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


void realizarComprobaciones(int condicionEjecutar,struct intr_frame *f UNUSED,int numeroGetArgs){

  switch(condicionEjecutar){
    case 1:
      // se va llenando el arreglo con la cantidad de argumentos que son necesarios
      get_args(f, &arg[0], numeroGetArgs);
      //valida si la linea del comando es valido
      verificarStringValido((const void *)arg[0]);
      // obtenemos el puntero de la pagina
      arg[0] = getpage_ptr((const void *) arg[0]);
    break;
  }

}


static void syscall_handler (struct intr_frame *f UNUSED)
{
  if (!FILE_LOCK_INIT) {
    lock_init(&file_system_lock);
    FILE_LOCK_INIT = true;
  }


  int esp = getpage_ptr((const void *) f->esp);

  switch (* (int *)esp) {
    case SYS_HALT:
      /*
        Segun Stanford, solo se debe utilizar pocas veces
        Detenemos pintos "Apagando la computadora"

      */

      shutdown_power_off(); // from shutdown.h

      break;

    case SYS_CREATE:
      realizarComprobaciones(1,f,2);

      /* creamos el syscall le mandamos el nombre del archivo y el tamanio sin signo */
      f->eax = syscall_create((const char *)arg[0], (unsigned)arg[1]);  // create this file
      break;

    case SYS_REMOVE:

      realizarComprobaciones(1,f,1);

      f->eax = syscall_remove((const char *)arg[0]);  // elimina este archivo
      break;

    case SYS_OPEN:
        realizarComprobaciones(1,f,1);
      /* creamos syscall_open(int filedes) */
      f->eax = syscall_open((const char *)arg[0]);  // abre este  archivo
      break;

    case SYS_FILESIZE:
      get_args(f, &arg[NUMERO0], NUMERO1);
      /* creamos syscall_filesize (const char *file_name) le enviamos el nombre del archcivo */
      f->eax = syscall_filesize(arg[NUMERO0]);  // obtenemos el tamanio del archivo
      break;

    case SYS_WRITE:
      get_args(f, &arg[NUMERO0], NUMERO3);

      /* verificamos si el buffer es valido
       * no queremos tener un buffer que esta fuera de nuestra memoria virtual
       */
       validate_buffer((const void*)arg[NUMERO1], (unsigned)arg[NUMERO2]);
      arg[NUMERO1] = getpage_ptr((const void *)arg[NUMERO1]);

      /* creamos syscall_write (int filedes, const void * buffer, unsigned bytes)*/
      f->eax = syscall_write(arg[NUMERO0], (const void *) arg[NUMERO1], (unsigned) arg[NUMERO2]);
      break;

    case SYS_TELL:
      get_args(f, &arg[NUMERO0], NUMERO1);
      /*  creamos syscall_tell(int filedes) */
      f->eax = syscall_tell(arg[NUMERO0]);
      break;

    case SYS_EXIT:
      get_args(f, &arg[NUMERO0], NUMERO1);
      syscall_exit(arg[NUMERO0]);
      break;

    case SYS_EXEC:
      realizarComprobaciones(1,f,1);

      f->eax = syscall_exec((const char*)arg[NUMERO0]); // Ejecuta la linea de comando

      break;

    case SYS_READ:
      get_args(f, &arg[NUMERO0], NUMERO3);

       // verifica el buffer sea valido
       validate_buffer((const void*)arg[NUMERO1], (unsigned)arg[NUMERO2]);
      // obtenemos el puntero de la pagina
      arg[NUMERO1] = getpage_ptr((const void *)arg[NUMERO1]);

      /* creamos  syscall_write (int filedes, const void * buffer, unsigned bytes)*/
      f->eax = syscall_read(arg[NUMERO0], (void *) arg[NUMERO1], (unsigned) arg[NUMERO2]);
      break;

    case SYS_SEEK:
      get_args(f, &arg[NUMERO0], NUMERO2);
      /* creamos  syscall_seek(int filedes, unsigned new_position) */
      syscall_seek(arg[NUMERO0], (unsigned)arg[NUMERO1]);
      break;

    case SYS_WAIT:
      get_args(f, &arg[NUMERO0], NUMERO1);
      f->eax = syscall_wait(arg[NUMERO0]);
      break;

    case SYS_CLOSE:
      get_args (f, &arg[NUMERO0], NUMERO1);
      /* creamos syscall_close(int filedes) */
      syscall_close(arg[NUMERO0]);
      break;

    default:
      break;
  }
}
/* halt */
void
syscall_halt (void)
{
  shutdown_power_off(); // from shutdown.h
}


/*
Termina el programa de usuario actual, devolviendo el estado al kernel. Si el padre del proceso lo espera (
ver más abajo), este es el estado que se devolverá.
Convencionalmente, un estado de 0 indica éxito y los valores distintos de cero indican errores.
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

/*paginas : This function gets the pointer to the active page from the thread*/
int
getpage_ptr(const void *vaddr)
{
  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr) {
    syscall_exit(ERROR);
  }
  return (int)ptr;
}

//argumentos
void
get_args (struct intr_frame *f, int *args, int num_of_args)
{
  int i=0;
  int *ptr;

  while (i< num_of_args) {
    ptr = (int *) f->esp + i + 1;
    validate_ptr((const void *) ptr);
    args[i] = *ptr;
    i++;
  }
}


/* buffer: función para comprobar si el búfer es válido */
void
validate_buffer(const void* buf, unsigned byte_size)
{
  unsigned i = 0;
  char* local_buffer = (char *)buf;
  for (; i < byte_size; i++)
  {
    validate_ptr((const void*)local_buffer);
    local_buffer++;
  }
}


/*punteros: This function validates the pointer*/
void
validate_ptr (const void *vaddr)
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

/*string: función para comprobar que la cadena es válida */
void
verificarStringValido (const void* str)
{
    for (; * (char *) getpage_ptr(str) != 0; str = (char *) str + 1);
}



/*files: close the desired file descriptor */
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

/*
Lee el tamaño de bytes del archivo abierto como fd en el búfer.
Devuelve el número de bytes realmente leídos (0 al final del archivo), o -1 si el archivo no se pudo leer
(debido a una condición que no sea el final del archivo). Fd 0 lee desde el teclado usando input_getc ().

*/

int
syscall_read(int filedes, void *buffer, unsigned length)
{
  if (length <= 0)
  {
    return length;
  }

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
  return NULL;
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


/*chil process: Remove all the child processes in a thread */
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


/*bool tipoAccionSyscall(int opcionEjecutar){
  lock_acquire(&file_system_lock);
  bool successful = false; // from filesys.h
  lock_release(&file_system_lock);
  return successful;
  switch(opcionEjecutar){

    case 1:
        /*Elimina el archivo llamado archivo. Devuelve verdadero si tiene éxito, falso en caso contrario.
    Un archivo puede eliminarse independientemente de si está abierto o cerrado, y la eliminación de un
    archivo abierto no lo cierra. Consulte Eliminación de un archivo abierto para obtener más detalles.

    successful = filesys_remove(file_name);

    break;

    case 2:


    break;

    default:
    break;
  }
}*/


/*  syscall: creamos syscall_create */
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

  verificarByteLeer(file_ptr);

  int filedes = add_file(file_ptr);
  lock_release(&file_system_lock);
  return filedes;
}

/*
agregar archivo a la lista de archivos y devolver el descriptor de archivo del archivo agregado*/
int add_file (struct file *file_name)
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

  verificarByteLeer(file_ptr);

  int filesize = file_length(file_ptr); // from file.h
  lock_release(&file_system_lock);
  return filesize;
}



/*
Segun Stanford: Escribe bytes de tamaño desde el búfer al archivo abierto fd. Devuelve el número de bytes realmente escritos,
que puede ser menor que el tamaño si no se pudieron escribir algunos bytes.
*/

int devolverNumeroBytes(unsigned byteSize,int fichero, int condicionComparar,const void * buffer){
  if (byteSize<=condicionComparar){
    return byteSize;
  }
  if (fichero == STD_OUTPUT)
  {
    putbuf (buffer, byteSize); // from stdio.h
    return byteSize;
  }
}


/* creamos syscall_write */
int  syscall_write (int filedes, const void * buffer, unsigned byte_size)
{
    devolverNumeroBytes(byte_size,filedes,0,buffer);
    // se empiez a escribir en el archivo
    lock_acquire(&file_system_lock);
    struct file *file_ptr = get_file(filedes);
    verificarByteLeer(file_ptr);

    int bytes_written = file_write(file_ptr, buffer, byte_size); // file.h
    lock_release (&file_system_lock);
    return bytes_written;
}

/*
 Devuelve la posición del siguiente byte a leer o escribir en el archivo abierto fd,
 expresado en bytes desde el principio del archivo.

*/

int verificarByteLeer( struct file *file_ptr){
  if (!file_ptr){
    lock_release(&file_system_lock);
    return ERROR;
  }
}


/* syscall_tell */
unsigned
syscall_tell(int filedes)
{
  lock_acquire(&file_system_lock);
  struct file *file_ptr = get_file(filedes);
  verificarByteLeer(file_ptr);

  off_t offset = file_tell(file_ptr); //from file.h
  return offset;
}