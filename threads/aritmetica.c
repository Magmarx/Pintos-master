#include <stdio.h>

#include "aritmetica.h"


// calculos planteados en web.stanford.edu


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
int multiplicar(int numberOne,int numberTwo){

	int result = numberTwo * numberOne;
	return result;
}


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

int divisionFraccion(int numberOne,int numberTwo){

	
	return ((int64_t) numberOne) * corrimientoIzquierda / numberTwo;
}

int convertToFixedPoint(int n){
	return n*corrimientoIzquierda;
}


