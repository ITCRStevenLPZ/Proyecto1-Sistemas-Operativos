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


//---------------------------------------------VARIABLES DE CADA LADO DEL PUENTE (TEMP)


int espera_semaforo_este = 5; //segundos de luz verde del semaforo de oeste

float interocurrencia_EW = 4; //media de la distribucion exponencial del tiempo entre llegadas de nuevos automoviles

int lim_inf_velocidad_EW = 60; //limite inferior de la velocidad de los autos

int lim_sup_velocidad_EW = 70; //limite superior de la velocidad de los autos

int velocidad_prom_EW = 7; //velocidad promedio de los autos

//----------------------------------------------------------------------------------


int espera_semaforo_oeste = 3; //segundos de luz verde del semaforo de este

float interocurrencia_WE = 5; //media de la distribucion exponencial del tiempo entre llegadas de nuevos automoviles

int lim_inf_velocidad_WE = 80; //limite inferior de la velocidad de los autos

int lim_sup_velocidad_WE = 85; //limite superior de la velocidad de los autos

int velocidad_prom_WE = 9; //velocidad promedio de los autos



//----------------------------------------------------------------------------------

int flag_turno = 0;
pthread_mutex_t TURNO = PTHREAD_MUTEX_INITIALIZER;

//int flag_libre = 0;
//pthread_mutex_t LIBRE = PTHREAD_MUTEX_INITIALIZER;

int cant_carros_puente_EW = 0;
pthread_mutex_t CARROS_PUENTE_EW = PTHREAD_MUTEX_INITIALIZER;

int cant_carros_puente_WE = 0;
pthread_mutex_t CARROS_PUENTE_WE = PTHREAD_MUTEX_INITIALIZER;

void CARNAGE_MUTEX(int velocidad_carro) {
   double tiempo_en_pasar = ((double) 0.001 / (double)velocidad_carro)*3600000; //es 0.001

   //printf("TIEMPO = %lfms, con velocidad de %d km/hr\n",tiempo_en_pasar, velocidad_carro);
   usleep((int)tiempo_en_pasar*1000);
}

pthread_cond_t COND_HILOS_CARROS = PTHREAD_COND_INITIALIZER;

void *CARRO_THREAD_EW(void *param){
   pthread_mutex_t CARRO_TEMP = PTHREAD_MUTEX_INITIALIZER;
   struct CARRO* car = (struct CARRO*) param;
   pthread_mutex_lock(&CARRO_TEMP);
   while(1){
      if(flag_turno == 0){
         pthread_mutex_lock(&TURNO);
         flag_turno = TURNO_EW; 
         pthread_mutex_unlock(&TURNO);
      }
      if(flag_turno == TURNO_EW){
         printf("EW - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = CANT_MUTEX-1; i >= 0; i--){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            CARNAGE_MUTEX(car->velocidad);

            pthread_mutex_lock(&CARROS_PUENTE_EW);
            cant_carros_puente_EW--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_EW);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }

         if(cant_carros_puente_EW == 0){//basicamente este if va a ser ingresado por el ultimo carro, ya que si la cantidad de carros en el puente es 0, quiere decir que ya no hay ninguno en el puente, y pone el turno PARA EL PRIMERO QUE LLEGUE
            pthread_mutex_lock(&TURNO);
            flag_turno = 0; 
            pthread_cond_broadcast(&COND_HILOS_CARROS); //despierta a todos los carros o hilos dormidos o pausados
            pthread_mutex_unlock(&TURNO);
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         printf("EW - ESPERA UN CARRO, NO ES EL TURNO\n");
         pthread_cond_wait(&COND_HILOS_CARROS, &CARRO_TEMP);//si no es el turno de el, el carro (el thread) se detiene
         printf("EW - DESPIERTA UN CARRO, POSIBLEMENTE ES SU TURNO\n");
      }
   }
   pthread_mutex_unlock(&CARRO_TEMP);
   printf("EW - SALE CARRO ID = %d\n", car->id);
   free(car);
   pthread_mutex_destroy(&CARRO_TEMP);
}


void *CARRO_THREAD_WE(void *param){
   printf("WE - LLEGO UN CARRO\n");
   pthread_mutex_t CARRO_TEMP = PTHREAD_MUTEX_INITIALIZER;
   struct CARRO* car = (struct CARRO*) param;
   pthread_mutex_lock(&CARRO_TEMP);
   while(1){
      if(flag_turno == 0){
         pthread_mutex_lock(&TURNO);
         flag_turno = TURNO_WE; 
         pthread_mutex_unlock(&TURNO);
      }
      if(flag_turno == TURNO_WE){
         printf("WE - ENTRA CARRO ID = %d\n", car->id);
         int i;
         for(i = 0; i < CANT_MUTEX; i++){
            pthread_mutex_lock(&PUENTE[i]);//comienza a usar ese mutex en especifico, o ese pedazo de puente

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE++; //Suma a la cantidad de carros que usan el puente por cada mutex que se va ingresando
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            //printf("ENTRA CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
            CARNAGE_MUTEX(car->velocidad);

            pthread_mutex_lock(&CARROS_PUENTE_WE);
            cant_carros_puente_WE--; //Resta a la cantidad de carros que usan el puente por cada mutex que se va saliendo
            pthread_mutex_unlock(&CARROS_PUENTE_WE);

            pthread_mutex_unlock(&PUENTE[i]);//deja de usar ese mutex en especifico, o ese pedazo de puente
            //printf("SALE CARRO ID = %d, Y MUTEX EN QUE PASA = %d\n", car->id, i);
         }

         if(cant_carros_puente_WE == 0){//basicamente este if va a ser ingresado por el ultimo carro, ya que si la cantidad de carros en el puente es 0, quiere decir que ya no hay ninguno en el puente, y pone el turno PARA EL PRIMERO QUE LLEGUE
            pthread_mutex_lock(&TURNO);
            flag_turno = 0; 
            pthread_cond_broadcast(&COND_HILOS_CARROS); //despierta a todos los carros o hilos dormidos o pausados
            pthread_mutex_unlock(&TURNO);
         }
         break; //sale del while loop, porque ya paso el puente
      }else{
         printf("WE - ESPERA UN CARRO, NO ES EL TURNO\n");
         pthread_cond_wait(&COND_HILOS_CARROS, &CARRO_TEMP);//si no es el turno de el, el carro (el thread) se detiene
         printf("WE - DESPIERTA UN CARRO, POSIBLEMENTE ES SU TURNO\n");
      }
   }
   pthread_mutex_unlock(&CARRO_TEMP);
   printf("WE - SALE CARRO ID = %d\n", car->id);
   free(car);
   pthread_mutex_destroy(&CARRO_TEMP);
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
         double r = (double)rand() / (double)RAND_MAX ;
         double tiempo = (- interocurrencia_EW * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
         int int_tiempo = (int) tiempo;
         printf("EW - Esperando %ds a que llegue el carro\n", int_tiempo);
         sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
         pthread_create(&CARROS_EW[i],NULL,CARRO_THREAD_EW,nuevo);
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
         double r = (double)rand() / (double)RAND_MAX ;
         double tiempo = (- interocurrencia_WE * log(1-r)); //se encarga de retornar un tiempo en ms, de ocurrencia, siguiendo una distribucion exponencial
         int int_tiempo = (int) tiempo;
         printf("WE - Esperando %ds a que llegue el carro\n", int_tiempo);
         sleep(int_tiempo);  //el hilo creador espera el tiempo de ocurrencia retornado
         pthread_create(&CARROS_WE[i],NULL,CARRO_THREAD_WE,nuevo);
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
   pthread_create(&cEW,NULL,spawnear_carros_EW,NULL); //primero se llama a los hilos creadores de hilos de autos
   pthread_create(&cWE,NULL,spawnear_carros_WE,NULL);

   pthread_join(cEW,NULL);
   pthread_join(cWE,NULL);





   return 1;
}

