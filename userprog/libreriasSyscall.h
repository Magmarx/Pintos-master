/*

    Prototipos que se debe implementar en syscall.c
*/

#define MAX_ARGS 3

/* syscall_read */
#define STD_INPUT 0
#define STD_OUTPUT 1

void get_args (struct intr_frame *f, int *arg, int num_of_args);

void validate_ptr (const void* vaddr);
int add_file (struct file *file_name);
void validate_str (const void* str);
void validate_buffer (const void* buf, unsigned byte_size);
static void syscall_handler (struct intr_frame *);
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