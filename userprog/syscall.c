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

// Inicializamos para que no tenga contenido basura

int contador = 0;
int *puntero = 0;

bool FILE_LOCK_INIT = false;


struct lock memoriaDeLock;

void syscall_init (void) 
{
  lock_init(&memoriaDeLock);
  intr_register_int (DIRECCION_0X30,TOKEN3, INTR_ON, syscall_handler, SYSCALL);
}

void realizarVerificacionesSyscall(struct intr_frame *intr_f,int numeroPosicion,int arg[], int opcionEjecutar){
  
  switch(opcionEjecutar){
    case 1:
      // se va llenando el arreglo con la cantidad de argumentos que son necesarios 
      obtenerArgumentos(intr_f, &arg[NUMERO0], numeroPosicion);
      
      //valida si la linea del comando es valido
      validate_str((const void *)arg[NUMERO0]);
      
      // obtenemos el puntero de la pagina
      arg[NUMERO0] = getpage_ptr((const void *) arg[NUMERO0]);
      
    break;
  }

}

static void syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!FILE_LOCK_INIT) {
    lock_init(&file_system_lock);
    FILE_LOCK_INIT = true;
  }
  
  int arg[argumentoMaximo];
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
      realizarVerificacionesSyscall(f,NUMERO2,argumentoMaximo,1);    
      f->eax = syscallCreacion((const char *)arg[0], (unsigned)arg[1]);  // create this file
      break;

    case SYS_REMOVE:

      realizarVerificacionesSyscall(f,NUMERO1,argumentoMaximo,1);    
      f->eax = syscallEliminar((const char *)arg[0]); 
      break;

    case SYS_OPEN:
      realizarVerificacionesSyscall(f,NUMERO1,argumentoMaximo,1);    
      /* creamos syscallAbrir(int filedes) */
      f->eax = syscallAbrir((const char *)arg[0]);  // abre este  archivo 
      break;

    case SYS_FILESIZE:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[0], 1);
      
      f->eax = syscallTamanoArchivo(arg[0]);  // obtenemos el tamanio del archivo
      break;
    
    case SYS_WRITE:
      
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[0], 3);
      
      verificarBuffer((const void*)arg[1], (unsigned)arg[2]);
       
      // obtenemos el puntero de pagina 
      arg[1] = getpage_ptr((const void *)arg[1]); 
      
      /* creamos syscallEscritura (int filedes, const void * buffer, unsigned bytes)*/
      f->eax = syscallEscritura(arg[0], (const void *) arg[1], (unsigned) arg[2]);
      break;
    
    case SYS_TELL:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[0], 1);
      /*  creamos syscall_tell(int filedes) */
      f->eax = syscall_tell(arg[0]);
      break;
    
  
    case SYS_EXIT:
      obtenerArgumentos(f, &arg[0], 1);
      syscall_exit(arg[0]);
      break;

    case SYS_EXEC:

      realizarVerificacionesSyscall(f,NUMERO1,argumentoMaximo,1);    
      f->eax = syscall_exec((const char*)arg[0]); // Ejecuta la linea de comando 

      break;

    case SYS_READ:
      obtenerArgumentos(f, &arg[0], 3);
      
      verificarBuffer((const void*)arg[1], (unsigned)arg[2]);
      // obtenemos el puntero de la pagina 
      arg[1] = getpage_ptr((const void *)arg[1]); 
      
      /* creamos  syscallEscritura (int filedes, const void * buffer, unsigned bytes)*/
      f->eax = syscallLectura(arg[0], (void *) arg[1], (unsigned) arg[2]);
      break;

    case SYS_SEEK:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos(f, &arg[0], 2);
      /* creamos  syscall_seek(int filedes, unsigned new_position) */
      syscall_seek(arg[0], (unsigned)arg[1]);
      break;

    case SYS_WAIT:
      // fill arg with the amount of arguments needed
      obtenerArgumentos(f, &arg[0], 1);
      f->eax = syscall_wait(arg[0]);
      break;

    case SYS_CLOSE:
      // se va llenando el arreglo con la cantidad de argumentos necesarios 
      obtenerArgumentos (f, &arg[0], 1);
      /* creamos syscall_close(int filedes) */
      syscall_close(arg[0]);
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


void obtenerArgumentos (struct intr_frame *f, int *args, int num_of_args)
{
  int i;
  int *ptr;

  for (i = 0; i < num_of_args; i++) {
    ptr = (int *) f->esp + i + 1;
    verificarPuntero((const void *) ptr);
    args[i] = *ptr;
  }
}


/*buffer: función para comprobar si el búfer es válido */
void
verificarBuffer(const void* buf, unsigned byte_size)
{
  unsigned i = 0;
  char* local_buffer = (char *)buf;
  for (; i < byte_size; i++)
  {
    verificarPuntero((const void*)local_buffer);
    local_buffer++;
  }
}


/*se verifica que el puntero sea valido */
void
verificarPuntero (const void *vaddr)
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



// cerramos el archivo
void process_close_file (int file_descriptor)
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



int
syscallLectura(int filedes, void *buffer, unsigned length)
{
  if (length <= 0)
  {
    return length;
  }
  
  if (filedes == valorCero)
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
void remove_child_process (struct child_process *cp)
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

/*  creamos syscallCreacion */
bool syscallCreacion(const char* file_name, unsigned starting_size)
{
  return creacionEliminacionSyscall(true,file_name,starting_size);
}
/*  creamos syscallEliminar */
bool syscallEliminar(const char* file_name)
{

  return creacionEliminacionSyscall(true,file_name,0);
}

void creacionEliminacionSyscall(bool condicion,const char* file_name,unsigned starting_size){
  lock_acquire(&file_system_lock);
  // remueve 
  if(condicion==true){
    bool ejecucion = filesys_remove(file_name);
    return ejecucion;
  
  }else{
    //crea
    bool ejecucion = filesys_create(file_name, starting_size); // from filesys.h
    lock_release(&file_system_lock);
    return ejecucion;
  }
  return condicion;
}

/* cramos syscallAbrir */
int syscallAbrir(const char *file_name)
{
  lock_acquire(&file_system_lock);
  struct file *file_ptr = filesys_open(file_name); // from filesys.h
  verificadorSyscall(file_ptr);
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

/* creamos  syscallTamanoArchivo */
int syscallTamanoArchivo(int filedes)
{
  lock_acquire(&file_system_lock);
  struct file *file_ptr = get_file(filedes);
  verificadorSyscall(file_ptr);
  int filesize = file_length(file_ptr); // from file.h
  lock_release(&file_system_lock);
  return filesize;
}

/* creamos syscallEscritura */
int  syscallEscritura (int filedes, const void * buffer, unsigned byte_size)
{
    if (byte_size <= 0)
    {
      return byte_size;
    }
    if (filedes == valor1)
    {
      putbuf (buffer, byte_size); // from stdio.h
      return byte_size;
    }
    
    // se empiez a escribir en el archivo 
    lock_acquire(&file_system_lock);
    struct file *file_ptr = get_file(filedes);
    verificadorSyscall(file_ptr);

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

  verificadorSyscall(file_ptr);

  off_t offset = file_tell(file_ptr); //from file.h
  return offset;
}


int verificadorSyscall(struct file *ficheroPuntero){
    if (!ficheroPuntero)
  {
    lock_release(&file_system_lock);
    return ERROR;
  }
}