#define TOKEN3 3
#define DIRECCION_0X30 0X30
#define SYSCALL "syscall"
#define VACIO NULL
#define TOKEN_V true
#define TOKEN_F false

#define NUMERO0 0
#define NUMERO1 1

#define NUMERO2 (32>>4)

/* syscallLectura */
#define valorCero 0
#define valor1 1


void obtenerArgumentos (struct intr_frame *f, int *arg, int num_of_args);
void verificarPuntero (const void* vaddr);

int add_file (struct file *file_name);

void validate_str (const void* str);
void verificarBuffer (const void* buf, unsigned byte_size);

// llamadas al sistema
static void syscall_handler (struct intr_frame *);
void syscall_halt (void);
void syscall_init (void);
void syscall_seek (int filedes, unsigned new_position);
void syscall_close(int filedes);
unsigned syscall_tell(int fildes);
int syscallLectura(int filedes, void *buffer, unsigned length);
int syscallAbrir(const char * file_name);
int syscallTamanoArchivo(int filedes);
int syscallEscritura (int filedes, const void * buffer, unsigned byte_size);
bool syscallCreacion(const char* file_name, unsigned starting_size);
bool syscallEliminar(const char* file_name);
pid_t syscall_exec(const char* cmdline);
