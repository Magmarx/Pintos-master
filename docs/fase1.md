## Fase #1 - Threads

Esta fase se divide en 3 partes

## Alarm Clock
En la carpeta **src/threads/** los siguientes archivos son necesarios para modificar el **Alarm Clock**:
~~~
thread.c
thread.h
~~~

### Problema inicial
Inicialmente el método **timer_sleep** tiene la siguiente implementación.
~~~
void
timer_sleep (int64_t ticks) 
{
  int64_t start = timer_ticks ();

  ASSERT (intr_get_level () == INTR_ON);
  while (timer_elapsed (start) < ticks) 
    thread_yield ();
}
~~~
Está implementación se conoce como **"busy waiting"**, en está implementación se revisa si el hilo debe estar durmiendo, al estarlo le da su lugar a otro hilo, **thread_yield()**, por lo tanto se calanderiza un nuevo hilo al procesador.

## Métodos agregados en Alarm Clock
En la siguiente imagen fueron agregados 2 métodos en el archivo:
~~~
thread.c
~~~
<img style="padding-top:30px;width:60%;height:50%;" src="images-fase1/agregarListaHilos.png" alt="Método agregar lista hilos en archivo thread.c">
<img style="padding-top:30px;width:60%;height:50%;" src="images-fase1/eliminarHiloDormido.png" alt="Método eliminar hilos dormidos en archivo thread.c">

En el archivo 
~~~
thread.h
~~~
Se agregaron los mismos métodos como prototipos. **Esto es para indicar que son métodos que se tienen que implementar. **
<img style="padding-top:30px;width:60%;height:50%;" src="images-fase1/prototipos.png" alt="Prototipos">


### Priority Scheduling 

### Advanced Scheduling
Por el momento no hemos implementado nada para Advanced scheduling ya que son puntos extras pero si tenemos pensado ver si podemos implementarlo mas adelante
