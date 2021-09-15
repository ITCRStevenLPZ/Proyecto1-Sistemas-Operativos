//Proyecto1 SO (Ronald Esquivel, Ricardo Murillo y Dylan Gonzalez)
/* Header */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

struct SEMAFORO {
	int time; //tiempo de duracion de la luz verde del semaforo
};

typedef struct CARRO //estructura de datos que representa a los carros de una direccion respectiva
{
	int velocidad; //parametro en km/h de la velocidad del automovil
	int es_ambulancia; // variable que define si el carro es ambulancia (1) o no (0)
	int id;
	
}CARRO;
