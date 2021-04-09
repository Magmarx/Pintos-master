## Fase #1 - Threads

[Home](https://magmarx.github.io/Pintos-master/)

Dentro de esta fase tuvimos que extender sobre la implementación de threads que nos dieron dentro del codigo de base de Pintos

Dentro del codigo base tenemos una estructura para los threads proporcionada por pintos, esta se encuentra dentro del archivo **thread.h**

```c
thread.h
struct thread 
{
 tid_t tid;                          /* El id del thread este nos sirve como identificador unico. */
 enum thread_status status;          /* El estado del thread. */
 char name[16];                      /* Nombre del thread (se usa para debbuggear) */
 uint8_t *stack;                     /* El puntero del stack del thread. */
 int priority;                       /* La prioridad del thread. */
 struct list_elem allelem;           /* Listado de elementos del thread. */

 /* Estas propiedades estan compartidas entre thread.c y synch.c. */
 struct list_elem elem;              /* Listado de los elementos que tiene el thread. */
}
```

Dentro de thread.h tambien vienen unas funciones por defecto las cuales son:

```c
thread.h

void thread_init (void); /* Inicializa las estructuras globales que todos los threads utilizan, como locks o semáforos. */
tid_t thread_create (const char *name, int priority, thread_func *, void *); /*Crea un nuevo thread y lo coloca en un ready state.*/
void thread_block (void); /* Bloquea al thread actual en ejecución */
void thread_unblock (struct thread *); /* Toma al thread que se pasa como argumento y lo desbloquea. */
struct thread *thread_current (void); /* Retorna un puntero al thread actual en ejecución. */
void thread_foreach (thread_action_func *, void *); /* Toma la función pasada como argumento y la aplica sobre cada thread. */
int thread_get_priority (void); /* Retorna la prioridad del thread actual en ejecución (Implementada en la  Priority Scheduling). */
void thread_set_priority (int); /* Toma el entero pasado como argumento, y cambia la prioridad del thread actual en ejecución al valor de este entero. (Implementada en la  Priority Scheduling). */

```

La linea de ejecución de los threads dentor de pintos es la siguiente:

![](./images-fase1/ThreadsMap.png)

Esta fase se divide en 3 partes

## Alarm Clock
En la carpeta **src/threads/** los siguientes archivos son necesarios para modificar el **Alarm Clock**:
```c
thread.c
thread.h
```

### Problema inicial
Inicialmente el método **timer_sleep** tiene la siguiente implementación.
```c
void
timer_sleep (int64_t ticks) 
{
  int64_t start = timer_ticks ();

  ASSERT (intr_get_level () == INTR_ON);
  while (timer_elapsed (start) < ticks) 
    thread_yield ();
}
```
Está implementación se conoce como **"busy waiting"**, en está implementación se revisa si el hilo debe estar durmiendo, al estarlo le da su lugar a otro hilo, **thread_yield()**, por lo tanto se calanderiza un nuevo hilo al procesador.

## Métodos agregados en Alarm Clock
En la siguiente imagen fueron agregados 2 métodos en el archivo:
~~~
thread.c
~~~

```c
void agregarListaHilosEspera(int64_t ticks){
  /*interrup.c, necesario para deshabilitar interrupciones*/
  enum intr_level antiguoNivel;

  /* Archivo threads/interrupt.c Disables interrupts and returns the previous interrupt status. */
  antiguoNivel = intr_disable();

  //retorna un puntero al hilo actual
  struct thread *hiloActual = thread_current();

  /* remueva el thread que se enuentra en estado ready list del struct
    list threadsEsperando. Realiza un cambio de estado, a thread blocked
  */

  //timer_ticks() Returns the number of timer ticks since the OS booted.
  // tiempoThreadDormido de thread.h
  hiloActual->tiempoThreadDormido = timer_ticks()+ticks;

  list_push_back(&threadsEsperando,&hiloActual->elem);

  // bloquea al thread actual en ejecucion
  thread_block();

  // habilitar interrupciones
  intr_set_level(antiguoNivel);
}
```


```c
/*
cuando sucede un timer_intept, si el tiempo del hilo ha terminado, se mueve de regreso a ready_list
con la funcion thread_unblock
*/

void eliminarHiloDormido(int64_t ticks){

  struct list_elem *iteracion = list_begin(&threadsEsperando);

  while(iteracion != list_end(&threadsEsperando)){
    struct thread *threadsListaEsperando = list_entry(iteracion,struct thread,elem);

    if (ticks>=threadsListaEsperando->tiempoThreadDormido)
    {
      /* code */
      // quitarlo de lista espera, regresarlo a ready_list
      iteracion=list_remove(iteracion);
      thread_unblock(threadsListaEsperando);
    }else{
      iteracion = list_next(iteracion);
    }
  }
}
```

En el archivo 
~~~
thread.h
~~~
Se agregaron los mismos métodos como prototipos. **Esto es para indicar que son métodos que se tienen que implementar.**

```c
/* Prototipo necesario para reimplementar el metodo timer_sleep del archivo timer.c
  para solucionar el busy waiting inicial
  agregarListaHilosEspera(ticks), lo que hace es bloquear al hilo actual, insertarlo en una lista 
  de hilos en espera de cumplir el tiempo que deben estar dormidos.
*/
void agregarListaHilosEspera(int64_t ticks);

/*prototipo que desbloquea los hilos que estan durmiendo luego de cierto tiempo. Implementacion en thread.c*/
void eliminarHiloDormido(int64_t numeroTicks);

```

## ¿Donde mandamos a llamar los métodos implementados?
Asumiendo que estamos dentro de nuestra carpeta principal llamada **Pintos-master**, en la carpeta  **devices**. El nombre del archivo a modificar es:
~~~
timer.c
~~~

El archivo **timer.c** allí se mandan a llamar los métodos implementados para eliminar el **busy waiting**



## ¿Qué encontramos en la carpeta devices?
Código fuente para la interfaz del dispositivo de E / S: teclado, temporizador, disco, etc. **Se utiliza para la implementación del temporizador en el proyecto 1.**

**En el archivo timer.h, el temporizador del sistema que marca, de forma predeterminada, 100**

En el método **timer_sleep**
```c
/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on. */
void
timer_sleep (int64_t ticks) 
{
  /*CODIGO original realiza busy waiting: 
  int64_t start = timer_ticks ();
  ASSERT (intr_get_level () == INTR_ON);
  while (timer_elapsed (start) < ticks) 
    thread_yield ();*/
  ASSERT (intr_get_level () == INTR_ON);
  agregarListaHilosEspera(ticks);
}
```

En el método **timer_interrupt**
```c
/* Timer interrupt handler. 
  timer_interrupt es el reloj de pintos. Al ser invocado la 
*/
static void
timer_interrupt (struct intr_frame *args UNUSED)
{
  ticks++;
  thread_tick ();

  // desbloque los hilos luego de cumplirse su tiempo
  eliminarHiloDormido(ticks);
}
```


**Recomendaciones para no alterar los resultados del autograder**
El argumento de timer_sleep () se expresa en tics del temporizador, no en milisegundos ni en ninguna otra unidad. Hay tics del temporizador TIMER_FREQ por segundo, donde **TIMER_FREQ es una macro definida en devices / timer.h.** El valor predeterminado es 100. **No es recomendable cambiar este valor, ya que es probable que cualquier cambio provoque la falla de muchas de las pruebas.**


### Priority Scheduling 

Dentro de esta fase tuvimos que implementar el calendarizador de tareas dentro de pintos para poder ordenar la prioridad con la que se ejecutan los diferentes threads.

Sabemos que Pintos cuenta con una calendarizacion ya implementada dentro de su codigo base la cual se basa en Round-Robin, El cual le asigna a cada thread un tiempo denominado quantum que es equivalente a 4 ticks por default cabe resaltar que en este metodo no existen  prioridades y que todos  los threads son tratados de forma  equitativa.

Trabajamos dentro de 4 archivos principalmente, estos se encuentran sobre pintos-master/src/threads/:

~~~
* thread.h
* thread.c
* synch.h
* synch.c
~~~





### Advanced Scheduling
Por el momento no hemos implementado nada para Advanced scheduling ya que son puntos extras pero si tenemos pensado ver si podemos implementarlo mas adelante
