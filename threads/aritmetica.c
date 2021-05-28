#include <stdio.h>

#include "aritmetica.h"


/* calculos planteados en web.stanford.edu

		4.4 BSD Scheduler
*/

/*

	1<<14


	1
	  inicialmente hay
	  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1     0x1

	14
	  0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0     0x4000


*/

#define corrimientoIzquierda 1<<14

// Implementacion de prototipos establecidos en aritmetica.h

// x by n
int multiplicar(int numberOne,int numberTwo){

	int result = numberTwo * numberOne;
	return result;
}

// divide x by n
int division(int numberOne,int numberTwo){

	int result = numberTwo / numberOne;
	return result;
}

//add x and y
int suma(int numberOne,int numberTwo){

	int result = numberTwo + numberOne;
	return result;
}

// resta y de x
int resta(int numberOne,int numberTwo){

	int result = numberOne - numberTwo;
	return result;
}

// divide x by y
int divisionFraccion(int numberOne,int numberTwo){

	
	return ((int64_t) numberOne) * corrimientoIzquierda / numberTwo;
}



int multiplicacionFraccion(int numberOne,int numberTwo){

	
	return ((int64_t) numberOne) * numberTwo / corrimientoIzquierda;
}

// convert n to fixed point
int convertNToFixedPoint(int n){
	return n*corrimientoIzquierda;
}


//Convert x to integer (rounding toward zero)
int convertXToInteger(int x){
	return x / corrimientoIzquierda;
}




// add x and n
int sumaFraccion(int x, int n){
	return x+(n * corrimientoIzquierda);
}


// convert x to integer, rounding to nearest
int redondeo(int x){
	if (x<=0)
	{
		/* code */
		return (x - corrimientoIzquierda / 2)/corrimientoIzquierda;
	}else{
		return (x + corrimientoIzquierda / 2)/corrimientoIzquierda;
	}
}