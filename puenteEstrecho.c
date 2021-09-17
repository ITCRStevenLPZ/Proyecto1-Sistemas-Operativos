#include "puenteEstrecho.h"

#define PORCENTAJE_AMBULANCIAS 0.1 //probabilidad de que el carro sea una ambulancia
#define CANT_MUTEX 100  //cantidad de mutex que se van a distribuir a lo largo del puente
#define NUM_CARROS 10 //numero de carros en la lista de threads por cada lado
#define AMBULANCIA_PORC 0.1 //porcentaje de autos que seran ambulancias
double longitud_puente = 100; //esta es la longitud del puente que es un parametro de entrada en m, CANT_MUTEX depende de este parametro

#define TURNO_EW 1
#define TURNO_WE 2

pthread_mutex_t PUENTE[CANT_MUTEX];

pthread_t CARROS_EW[NUM_CARROS]; //lista de 

pthread_t CARROS_WE[NUM_CARROS];

int TIPO_ADMINISTRACION = 3;

struct SEMAFORO datos_semaforo_WE;

struct SEMAFORO datos_semaforo_EW;

//---------------------------------------------VARIABLES DE CADA LADO DEL PUENTE (TEMP)


int espera_semaforo_este = 20; //segundos de luz verde del semaforo de oeste

float interocurrencia_EW = 12; //media de la distribucion exponencial del tiempo entre llegadas de nuevos automoviles

int lim_inf_velocidad_EW = 60; //limite inferior de la velocidad de los autos

int lim_sup_velocidad_EW = 70; //limite superior de la velocidad de los autos

int velocidad_prom_EW = 7; //velocidad promedio de los autos

int cant_carros_oficial_EW = 6;


//----------------------------------------------------------------------------------


int espera_semaforo_oeste = 15; //segundos de luz verde del semaforo de este

float interocurrencia_WE = 10; //media de la distribucion exponencial del tiempo entre llegadas de nuevos automoviles

int lim_inf_velocidad_WE = 80; //limite inferior de la velocidad de los autos

int lim_sup_velocidad_WE = 85; //limite superior de la velocidad de los autos

int velocidad_prom_WE = 9; //velocidad promedio de los autos

int cant_carros_oficial_WE = 6;

//----------------------------------------------------------------------------------

int flag_turno = 0;
pthread_mutex_t TURNO = PTHREAD_MUTEX_INITIALIZER;

//int flag_libre = 0;
//pthread_mutex_t LIBRE = PTHREAD_MUTEX_INITIALIZER;

int cant_carros_puente_EW = 0;
pthread_mutex_t CARROS_PUENTE_EW = PTHREAD_MUTEX_INITIALIZER;

int cant_carros_puente_WE = 0;
pthread_mutex_t CARROS_PUENTE_WE = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t cond_semaforo_WE = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_semaforo_WE = PTHREAD_MUTEX_INITIALIZER; 

pthread_cond_t cond_semaforo_EW = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_semaforo_EW = PTHREAD_MUTEX_INITIALIZER; 

//----------------------------------------------------------------------------------

void MUTEX(int velocidad_carro) {
   double tiempo_en_pasar = ((double) 0.001 / (double)velocidad_carro)*3600000; //es 0.001

   //printf("TIEMPO = %lfms, con velocidad de %d km/hr\n",tiempo_en_pasar, velocidad_carro);
   usleep((int)tiempo_en_pasar*1000);
}

pthread_cond_t COND_HILOS_CARROS_WE = PTHREAD_COND_INITIALIZER;

pthread_cond_t COND_HILOS_CARROS_EW = PTHREAD_COND_INITIALIZER;

pthread_mutex_t CARRO_TEMP_EW = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t CARRO_TEMP_WE = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t SEMAFORO_DEL_SEMAFORO = PTHREAD_MUTEX_INITIALIZER;

int estado_semaforo_EW = 0; //variable que indica si el semaforo esta en verde (0) o en rojo (1)

int estado_semaforo_WE = 1; //variable que indica si el semaforo esta en verde (0) o en rojo (1)

int carros_dentro_WE = 0;

int carros_dentro_EW = 0;

pthread_mutex_t CARRO_DENTRO_EW = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t CARRO_DENTRO_WE = PTHREAD_MUTEX_INITIALIZER;

int carros_oficial_EW;

int carros_oficial_WE;

pthread_mutex_t CARRO_OFICIAL_EW = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t CARRO_OFICIAL_WE = PTHREAD_MUTEX_INITIALIZER;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void *OFICIAL_THREAD_EW(void *param){
   struct CARRO* car = (struct CARRO*) param;
   double r = (double)rand() / (double)RAND_MAX ;
   double tiempo = (- interocurrencia_EW * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
   int int_tiempo = (int) tiempo;
   //printf("WE - Esperando %ds a que llegue el carro\n", int_tiempo);
   sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
   pthread_mutex_lock(&CARRO_TEMP_EW);
   /*pthread_mutex_lock(&CARRO_DENTRO_EW);
   carros_dentro_EW++;
   pthread_mutex_unlock(&CARRO_DENTRO_EW);*/
   //printf("EW - LLEGO UN CARRO ID = %d\n", car->id);
   while(1){   
      pthread_mutex_lock(&TURNO);
      if(flag_turno == 0){ 
         flag_turno = TURNO_EW; 
         printf("\n\n");
      }
      pthread_mutex_unlock(&TURNO);

      if(flag_turno == TURNO_EW && cant_carros_puente_WE == 0 && carros_oficial_EW != 0){

         pthread_mutex_lock(&CARRO_OFICIAL_EW);
         carros_oficial_EW--;
         printf("EW - OFICIAL: CARROS RESTANTES = %d\n",carros_oficial_EW);
         pthread_mutex_unlock(&CARRO_OFICIAL_EW);
         pthread_mutex_unlock(&CARRO_TEMP_EW);

         printf("EW - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = CANT_MUTEX-1; i >= 0; i--){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            MUTEX(car->velocidad);

            pthread_cond_signal(&COND_HILOS_CARROS_EW);

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }

         if(cant_carros_puente_EW == 0 || carros_oficial_EW == 0){//basicamente este if va a ser ingresado por el ultimo carro, ya que si la cantidad de carros en el puente es 0, quiere decir que ya no hay ninguno en el puente, y pone el turno PARA EL PRIMERO QUE LLEGUE
            printf("EW - NO HAY MAS CARROS DE ESTE LADO O YA SE CUMPLIO CON LA CANTIDAD QUE SE DEJA PASAR!, SE CEDE EL PASO!\n\n");
            pthread_mutex_lock(&TURNO);
            flag_turno = TURNO_WE; 
            pthread_mutex_unlock(&TURNO);
            pthread_mutex_lock(&CARRO_OFICIAL_EW);
            carros_oficial_EW = cant_carros_oficial_EW;
            pthread_mutex_unlock(&CARRO_OFICIAL_EW);
            pthread_cond_signal(&COND_HILOS_CARROS_WE); //despierta a los carros o hilos dormidos o pausados
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         //printf("WE - ESPERA UN CARRO, NO ES EL TURNO\n");
         pthread_cond_wait(&COND_HILOS_CARROS_EW, &CARRO_TEMP_EW);//si no es el turno de el, el carro (el thread) se detiene
         //printf("WE - DESPIERTA UN CARRO, POSIBLEMENTE ES SU TURNO\n");
         //pthread_mutex_unlock(&CARRO_TEMP_EW);
      }
   }
   /*pthread_mutex_lock(&CARRO_DENTRO_EW);
   carros_dentro_EW--;
   pthread_mutex_unlock(&CARRO_DENTRO_EW);*/
   printf("EW - SALE CARRO ID = %d\n", car->id);
   free(car);
}



void *OFICIAL_THREAD_WE(void *param){
   struct CARRO* car = (struct CARRO*) param;
   double r = (double)rand() / (double)RAND_MAX ;
   double tiempo = (- interocurrencia_WE * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
   int int_tiempo = (int) tiempo;
   //printf("WE - Esperando %ds a que llegue el carro\n", int_tiempo);
   sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
   pthread_mutex_lock(&CARRO_TEMP_WE);
   /*pthread_mutex_lock(&CARRO_DENTRO_WE);
   carros_dentro_WE++;
   pthread_mutex_unlock(&CARRO_DENTRO_WE);*/
   ///printf("WE - LLEGO UN CARRO\n");
   while(1){
      pthread_mutex_lock(&TURNO);
      if(flag_turno == 0){
         flag_turno = TURNO_WE;
         printf("\n\n"); 
      }

      pthread_mutex_unlock(&TURNO);
      if(flag_turno == TURNO_WE && cant_carros_puente_EW == 0 && carros_oficial_WE != 0){

         pthread_mutex_lock(&CARRO_OFICIAL_WE);
         carros_oficial_WE--;
         printf("WE - OFICIAL: CARROS RESTANTES = %d\n",carros_oficial_WE);
         pthread_mutex_unlock(&CARRO_OFICIAL_WE);

         pthread_mutex_unlock(&CARRO_TEMP_WE);
         printf("WE - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = 0; i < CANT_MUTEX; i++){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            MUTEX(car->velocidad);

            pthread_cond_signal(&COND_HILOS_CARROS_WE);

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }
         if(cant_carros_puente_WE == 0 || carros_oficial_WE == 0){//basicamente este if va a ser ingresado por el ultimo carro, ya que si la cantidad de carros en el puente es 0, quiere decir que ya no hay ninguno en el puente, y pone el turno PARA EL PRIMERO QUE LLEGUE
            printf("\n\nWE - NO HAY MAS CARROS DE ESTE LADO O YA SE CUMPLIO CON LA CANTIDAD QUE SE DEJA PASAR!, SE CEDE EL PASO!\n\n");
            pthread_mutex_lock(&TURNO);
            flag_turno = TURNO_EW; 
            pthread_mutex_unlock(&TURNO);
            pthread_mutex_lock(&CARRO_OFICIAL_WE);
            carros_oficial_WE = cant_carros_oficial_WE;
            pthread_mutex_unlock(&CARRO_OFICIAL_WE);
            pthread_cond_signal(&COND_HILOS_CARROS_EW); //despierta los carros o hilos dormidos o pausados
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         //printf("WE - ESPERA UN CARRO, NO ES EL TURNO\n");
         pthread_cond_wait(&COND_HILOS_CARROS_WE, &CARRO_TEMP_WE);//si no es el turno de el, el carro (el thread) se detiene
         //printf("WE - DESPIERTA UN CARRO, POSIBLEMENTE ES SU TURNO\n");
         //pthread_mutex_unlock(&CARRO_TEMP_WE);
      }
   }
   /*pthread_mutex_lock(&CARRO_DENTRO_WE);
   carros_dentro_WE--;
   pthread_mutex_unlock(&CARRO_DENTRO_WE);*/
   printf("WE - SALE CARRO ID = %d\n", car->id);
   free(car);
   
}
//--------------------------------------------------------------FUNCIONES EN DONDE CORREN LOS HILOS DE LOS SEMAFOROS --------------------------------------

void *semaforo_WE(void *tiempo_espera) { //esta funcion es la encargada de correr el semaforo , basicamente espera el tiempo indicado y avisa al otro semaforo que ya termino y se bloquea
   pthread_mutex_lock(&mutex_semaforo_WE);
   struct SEMAFORO* t = (struct SEMAFORO*) tiempo_espera;
   //printf("Hola, entre, tengo %ds de espera!\n", t->time);
   while(1){
      if(estado_semaforo_EW == 1){
         //printf("IN  EN W-E: semaforo_este_oeste = %d\n", estado_semaforo_EW);
         //printf("IN  EN W-E: semaforo_oeste_este = %d\n", estado_semaforo_WE);
         printf("SEMAFORO OESTE-ESTE EN VERDE POR %ds\n", t->time);
         sleep(t->time);
         pthread_mutex_lock(&SEMAFORO_DEL_SEMAFORO);
         estado_semaforo_WE = 1;
         estado_semaforo_EW = 0;
         //agregar un cond signal
         pthread_cond_signal(&COND_HILOS_CARROS_EW);
         pthread_cond_signal(&cond_semaforo_EW);
         pthread_mutex_unlock(&SEMAFORO_DEL_SEMAFORO);
         //printf("OUT EN W-E: semaforo_este_oeste = %d\n", estado_semaforo_EW);
         //printf("OUT EN W-E: semaforo_oeste_este = %d\n", estado_semaforo_WE);
      }else{
         //agregar un cond wait
         pthread_cond_wait(&cond_semaforo_WE, &mutex_semaforo_WE);
      }
   }
   pthread_mutex_unlock(&mutex_semaforo_WE);
}

void *semaforo_EW(void *tiempo_espera) {
   pthread_mutex_lock(&mutex_semaforo_EW);
   struct SEMAFORO* t = (struct SEMAFORO*) tiempo_espera;
   printf("Hola, entre, tengo %ds de espera!\n", t->time);
   while(1){
      if(estado_semaforo_WE == 1){
         //printf("IN  EN E-W: semaforo_este_oeste = %d\n", estado_semaforo_EW);
         //printf("IN  EN E-W: semaforo_oeste_este = %d\n", estado_semaforo_WE);
         printf("SEMAFORO ESTE-OESTE EN VERDE POR %ds\n", t->time);
         sleep(t->time);
         pthread_mutex_lock(&SEMAFORO_DEL_SEMAFORO);
         estado_semaforo_EW = 1;
         estado_semaforo_WE = 0;
         pthread_cond_signal(&COND_HILOS_CARROS_WE);
         //agregar un cond signal
         pthread_cond_signal(&cond_semaforo_WE);
         pthread_mutex_unlock(&SEMAFORO_DEL_SEMAFORO);
         //printf("OUT EN E-W: semaforo_este_oeste = %d\n", estado_semaforo_EW);
         //printf("OUT EN E-W: semaforo_oeste_este = %d\n", estado_semaforo_WE);
      }else{
         //agregar un cond wait
         pthread_cond_wait(&cond_semaforo_EW, &mutex_semaforo_EW);
      }
   }
   pthread_mutex_unlock(&mutex_semaforo_EW);
}

//-----------------------------------------------------------------------------------------------------------------------



void *SEMAFORO_THREAD_EW(void *param){
   struct CARRO* car = (struct CARRO*) param;
   double r = (double)rand() / (double)RAND_MAX ;
   double tiempo = (- interocurrencia_EW * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
   int int_tiempo = (int) tiempo;
   //printf("WE - Esperando %ds a que llegue el carro\n", int_tiempo);
   sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
   pthread_mutex_lock(&CARRO_TEMP_EW);
   //printf("EW - LLEGO UN CARRO ID = %d\n", car->id);
   while(1){   
      if(estado_semaforo_EW == 0 && cant_carros_puente_WE == 0){
         pthread_mutex_unlock(&CARRO_TEMP_EW);
         printf("EW - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = CANT_MUTEX-1; i >= 0; i--){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            MUTEX(car->velocidad);

            pthread_cond_signal(&COND_HILOS_CARROS_EW);

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }
         if(cant_carros_puente_EW == 0){
            pthread_cond_signal(&COND_HILOS_CARROS_WE);
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         //printf("EW - ESPERA UN CARRO, LA LUZ NO ESTA EN VERDE O PASA UN CARRO\n");
         pthread_cond_wait(&COND_HILOS_CARROS_EW, &CARRO_TEMP_EW);//si no es el turno de el, el carro (el thread) se detiene
         //printf("EW - DESPIERTA UN CARRO, POSIBLEMENTE ES SU TURNO\n");
         //pthread_mutex_unlock(&CARRO_TEMP_EW);
      }
      
   }
   printf("EW - SALE CARRO ID = %d\n", car->id);
   free(car);
}

void *SEMAFORO_THREAD_WE(void *param){
   struct CARRO* car = (struct CARRO*) param;
   double r = (double)rand() / (double)RAND_MAX ;
   double tiempo = (- interocurrencia_WE * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
   int int_tiempo = (int) tiempo;
   //printf("WE - Esperando %ds a que llegue el carro\n", int_tiempo);
   sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
   pthread_mutex_lock(&CARRO_TEMP_WE);
   ///printf("WE - LLEGO UN CARRO\n");
   while(1){
      if(estado_semaforo_WE == 0 && cant_carros_puente_EW == 0){
         pthread_mutex_unlock(&CARRO_TEMP_WE);
         printf("WE - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = 0; i < CANT_MUTEX; i++){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            MUTEX(car->velocidad);

            pthread_cond_signal(&COND_HILOS_CARROS_WE);

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }
         if(cant_carros_puente_WE == 0){
            pthread_cond_signal(&COND_HILOS_CARROS_EW);
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         //printf("WE - ESPERA UN CARRO, LA LUZ NO ESTA EN VERDE O PASA UN CARRO\n");
         pthread_cond_wait(&COND_HILOS_CARROS_WE, &CARRO_TEMP_WE);//si no es el turno de el, el carro (el thread) se detiene
         //printf("WE - DESPIERTA UN CARRO, POSIBLEMENTE ES LA LUZ ESTA EN VERDE\n");
         //pthread_mutex_unlock(&CARRO_TEMP_WE);
      }
   }
   
   printf("WE - SALE CARRO ID = %d\n", car->id);
   free(car);

   
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

void *CARNAGE_THREAD_EW(void *param){
   struct CARRO* car = (struct CARRO*) param;
   double r = (double)rand() / (double)RAND_MAX ;
   double tiempo = (- interocurrencia_EW * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
   int int_tiempo = (int) tiempo;
   //printf("WE - Esperando %ds a que llegue el carro\n", int_tiempo);
   sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
   pthread_mutex_lock(&CARRO_TEMP_EW);
   //printf("EW - LLEGO UN CARRO ID = %d\n", car->id);
   while(1){   
      pthread_mutex_lock(&TURNO);
      if(flag_turno == 0){ 
         flag_turno = TURNO_EW; 
         printf("\n\n");
      }
      pthread_mutex_unlock(&TURNO);
      if(flag_turno == TURNO_EW && cant_carros_puente_WE == 0){
         pthread_mutex_unlock(&CARRO_TEMP_EW);
         printf("EW - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = CANT_MUTEX-1; i >= 0; i--){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            MUTEX(car->velocidad);

            pthread_cond_signal(&COND_HILOS_CARROS_EW);

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }

         if(cant_carros_puente_EW == 0){//basicamente este if va a ser ingresado por el ultimo carro, ya que si la cantidad de carros en el puente es 0, quiere decir que ya no hay ninguno en el puente, y pone el turno PARA EL PRIMERO QUE LLEGUE
            pthread_mutex_lock(&TURNO);
            flag_turno = 0; 
            pthread_cond_signal(&COND_HILOS_CARROS_WE); //despierta a todos los carros o hilos dormidos o pausados
            pthread_mutex_unlock(&TURNO);
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         //printf("WE - ESPERA UN CARRO, NO ES EL TURNO\n");
         pthread_cond_wait(&COND_HILOS_CARROS_EW, &CARRO_TEMP_EW);//si no es el turno de el, el carro (el thread) se detiene
         //printf("WE - DESPIERTA UN CARRO, POSIBLEMENTE ES SU TURNO\n");
         //pthread_mutex_unlock(&CARRO_TEMP_EW);
      }
   }
   printf("EW - SALE CARRO ID = %d\n", car->id);
   free(car);
}


void *CARNAGE_THREAD_WE(void *param){
   struct CARRO* car = (struct CARRO*) param;
   double r = (double)rand() / (double)RAND_MAX ;
   double tiempo = (- interocurrencia_WE * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
   int int_tiempo = (int) tiempo;
   //printf("WE - Esperando %ds a que llegue el carro\n", int_tiempo);
   sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
   pthread_mutex_lock(&CARRO_TEMP_WE);
   ///printf("WE - LLEGO UN CARRO\n");
   while(1){
      pthread_mutex_lock(&TURNO);
      if(flag_turno == 0){
         flag_turno = TURNO_WE;
         printf("\n\n"); 
      }
      pthread_mutex_unlock(&TURNO);
      if(flag_turno == TURNO_WE && cant_carros_puente_EW == 0){
         pthread_mutex_unlock(&CARRO_TEMP_WE);
         printf("WE - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = 0; i < CANT_MUTEX; i++){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            MUTEX(car->velocidad);

            pthread_cond_signal(&COND_HILOS_CARROS_WE);

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }

         if(cant_carros_puente_WE == 0){//basicamente este if va a ser ingresado por el ultimo carro, ya que si la cantidad de carros en el puente es 0, quiere decir que ya no hay ninguno en el puente, y pone el turno PARA EL PRIMERO QUE LLEGUE
            pthread_mutex_lock(&TURNO);
            flag_turno = 0; 
            pthread_cond_signal(&COND_HILOS_CARROS_EW); //despierta a todos los carros o hilos dormidos o pausados
            pthread_mutex_unlock(&TURNO);
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         //printf("WE - ESPERA UN CARRO, NO ES EL TURNO\n");
         pthread_cond_wait(&COND_HILOS_CARROS_WE, &CARRO_TEMP_WE);//si no es el turno de el, el carro (el thread) se detiene
         //printf("WE - DESPIERTA UN CARRO, POSIBLEMENTE ES SU TURNO\n");
         pthread_mutex_unlock(&CARRO_TEMP_WE);
      }
   }
   
   printf("WE - SALE CARRO ID = %d\n", car->id);
   free(car);

   
}

void *spawnear_carros_EW(void *arg){//Esta funcion es recorida por un thread que crea los hilos que van a recorrer el puente
   while(1){
      int i;
      for(i = 0; i < NUM_CARROS; i++){
         int velocidad = (rand() % (lim_sup_velocidad_EW - lim_inf_velocidad_EW + 1)) + lim_inf_velocidad_EW; //es una velocidad random dentro de dos velocidades (inf y sup)
         struct CARRO *nuevo;
         nuevo = malloc(sizeof(struct CARRO));
         nuevo->velocidad = velocidad;
         if((double)rand() / (double)RAND_MAX  <= AMBULANCIA_PORC){ // dependiendo del numero random, si es menor o igual al la probabilidad de que sea una ambulancia, el thread va a representar a una ambulancia
            nuevo->es_ambulancia = 1;
         }else{
            nuevo->es_ambulancia = 0;
         }
         nuevo->id = i;

         if(TIPO_ADMINISTRACION == 1){
            pthread_create(&CARROS_EW[i],NULL,CARNAGE_THREAD_EW,nuevo);
         }else if(TIPO_ADMINISTRACION == 2){
            pthread_create(&CARROS_EW[i],NULL,SEMAFORO_THREAD_EW,nuevo);
         }else{
            pthread_create(&CARROS_EW[i],NULL,OFICIAL_THREAD_EW,nuevo);
         }     
      }

      for(i = 0; i < NUM_CARROS; i++){
         pthread_join(CARROS_EW[i],NULL);
      }
   }
}

void *spawnear_carros_WE(void *arg){ //Esta funcion es recorida por un thread que crea los hilos que van a recorrer el puente
   while(1){
      int i;
      for(i = 0; i < NUM_CARROS; i++){
         int velocidad = (rand() % (lim_sup_velocidad_WE - lim_inf_velocidad_WE + 1)) + lim_inf_velocidad_WE; //es una velocidad random dentro de dos velocidades (inf y sup)
         struct CARRO *nuevo;
         nuevo = malloc(sizeof(struct CARRO));
         nuevo->velocidad = velocidad;
         if((double)rand() / (double)RAND_MAX  <= AMBULANCIA_PORC){// dependiendo del numero random, si es menor o igual al la probabilidad de que sea una ambulancia, el thread va a representar a una ambulancia
            nuevo->es_ambulancia = 1;
         }else{
            nuevo->es_ambulancia = 0;
         }
         nuevo->id = i;

         if(TIPO_ADMINISTRACION == 1){
            pthread_create(&CARROS_WE[i],NULL,CARNAGE_THREAD_WE,nuevo);
         }else if(TIPO_ADMINISTRACION == 2){
            pthread_create(&CARROS_WE[i],NULL,SEMAFORO_THREAD_WE,nuevo);
         }else{
            pthread_create(&CARROS_WE[i],NULL,OFICIAL_THREAD_WE,nuevo);
         }
      }

      for(i = 0; i < NUM_CARROS; i++){
         pthread_join(CARROS_WE[i],NULL);
      }
   }
}



int main(int argc, char** argv){
   //srand(time(0));
   pthread_t fEW,fWE,cEW,cWE,sEW,sWE;

   int i;
   for(i = 0; i > CANT_MUTEX; i++){
      PUENTE[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
   }
   if(TIPO_ADMINISTRACION == 2){
      datos_semaforo_EW.time = espera_semaforo_este;
      datos_semaforo_WE.time = espera_semaforo_oeste;
  
      pthread_create(&sEW,NULL,semaforo_EW, &datos_semaforo_EW);
      pthread_create(&sWE,NULL,semaforo_WE, &datos_semaforo_WE);
   }else if(TIPO_ADMINISTRACION == 3){
      carros_oficial_EW = cant_carros_oficial_EW;
      carros_oficial_WE = cant_carros_oficial_WE;
   }

   pthread_create(&cEW,NULL,spawnear_carros_EW,NULL); //primero se llama a los hilos creadores de hilos de autos
   pthread_create(&cWE,NULL,spawnear_carros_WE,NULL);

   pthread_join(cEW,NULL);
   pthread_join(cWE,NULL);

   pthread_join(sEW,NULL);
   pthread_join(sWE,NULL);




   return 1;
}