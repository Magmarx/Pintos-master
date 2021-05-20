// Se ejecuta antes de compilar la libreria.


#ifdef aritmetica

#define aritmetica

#include <stdin.h>

typedef int32_t entero32;


// Fraccion fija, 14 bits por fraccion

/*
Desplazamiento a la izquierda

2<< 14
          2  
          	0 0 0 0 0 0 0 0  0 0 0 0 0 0 1 0  0x2

<<14  32768  
			1 0 0 0 0 0 0 0 0  0 0 0 0 0 0 0  0x8000
*/

#define fraccionFija (2<<14)


/*
	    2 00000000 00000010 0x2
	16384 01000000 00000000 0x4000

*/
#define mediaFraccionFija (2<<13)

#define INT2FIXED(n) ((n)*fraccionFija)




#else

#endif