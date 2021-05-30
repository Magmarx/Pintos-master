#define TOKEN3 3
#define DIRECCION_0X30 0X30
#define SYSCALL "syscall"
#define VACIO NULL
#define TOKEN_V true
#define TOKEN_F false

/* Son necesarios para el syscall_read */
#define STD_INPUT 0
#define STD_OUTPUT 1



/*Prototipos que se debe implmentr en syscall*/
static void syscall_handler (struct intr_frame *);

int obtenerLongitud(int longitud, int condicion);

void syscall_halt (void);
void syscall_init (void);
void syscall_seek (int filedes, unsigned new_position);
void syscall_close(int filedes);
unsigned syscall_tell(int fildes);
int syscall_read(int filedes, void *buffer, unsigned length);
int syscall_open(const char * file_name);
int syscall_filesize(int filedes);
int syscall_write (int filedes, const void * buffer, unsigned byte_size);
bool syscall_create(const char* file_name, unsigned starting_size);
bool syscall_remove(const char* file_name);
pid_t syscall_exec(const char* cmdline);



void getObtenerArgumentos (struct intr_frame *f, int *arg, int num_of_args);//args
void verificarPunteros(const void* vaddr);//pointers
int agregarArchivos (struct file *file_name);//file 
// Validator 
void validate_str (const void* str);
void verificadorBuffer (const void* buf, unsigned byte_size);

//args
void obtenerArgumentos (struct intr_frame *f, int *arg, int num_of_args);
//pointers
void validarPunteros (const void* vaddr);
//file 
int agregarArchivos (struct file *file_name);
// Validator 
void validate_str (const void* str);
void validarBuffer (const void* buf, unsigned byte_size);
