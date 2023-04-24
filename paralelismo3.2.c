#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 0

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/

#define M  1000000 // Number of sequences
#define N  200  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16) % 5;
}

// The distance between two bases
int base_distance(int base1, int base2){

  if((base1 == 4) || (base2 == 4)){
    return 3;
  }

  if(base1 == base2) {
    return 0;
  }

  if((base1 == 0) && (base2 == 3)) {
    return 1;
  }

  if((base2 == 0) && (base1 == 3)) {
    return 1;
  }

  if((base1 == 1) && (base2 == 2)) {
    return 1;
  }

  if((base2 == 2) && (base1 == 1)) {
    return 1;
  }

  return 2;
}

int main(int argc, char *argv[] ) {

    int i, j;
    int *partdata1, *partdata2;
    int *result;
    struct timeval  tv1, tv2, tv3, tv4,tv5, tv6;
    int size, rank;
    
    int *data1, *data2;//el proceso 0 inicializa las matrices completas y el array completo
    int *totalresult;// las declaramos aqui porque si no nos da fallo al compilar
    
    
    //Inicialización del entorno MPI
    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// tomamos el valor del numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);// tomamos el valor del rango de cada proceso   
    
    /* Initialize Matrices */
    if(rank == 0) { 
    data1 = (int *) malloc(M*N*sizeof(int));
    data2 = (int *) malloc(M*N*sizeof(int));
    totalresult = (int *) malloc(M*sizeof(int));
        for (i = 0; i < M; i++) {
            for (j = 0; j < N; j++) {
                /* random with 20% gap proportion */
                data1[i * N + j] = fast_rand();
                data2[i * N + j] = fast_rand();
            }
        }
    }
    int bloque = M/size; //calculamos el tamaño del bloque en filas
    int last = M-(bloque *(size-1));// calculamos el ultimo bloque en caso de que no sea divisible
    //todos los procesos realizan la reserva de espacio de la matriz bloque y reservamos los espacios
    if (rank!=(size-1)) { 
      partdata1 = (int *) malloc(bloque*N*sizeof(int));
      partdata2 = (int *) malloc(bloque*N*sizeof(int));
      result = (int *) malloc(bloque*sizeof(int));
    
    } else{//no se usara en este ejercicio dado que estamos probando con 4 procesos y M % 4 = 0
    partdata1 = (int *) malloc(last*N*sizeof(int));//comentamos el resto y dejamos inicializado el de mayor tamaño de momento
    partdata2 = (int *) malloc(last*N*sizeof(int));
    result = (int *) malloc(last*sizeof(int)); 
   }
     
//estamos realizando las mediciones de cada comunicacion por separado desde que se empieza a comunicar hasta que cada proceso recibe, tenemos la del calculo tambien por separado y luego recogemos el tiempo total, PREGUNTAR si el tiempo de comunicacion debe ser sumado para añadir calculo y una impresion solo al final (los print estan siendo añadidos al tiempo acordarse de comentarlo)
    //medición del tiempo de comunicación
    gettimeofday(&tv1, NULL);//medimos tiempo y almacenamos
    //printf("linea 102, rank %d\n", rank);
    //Envío de los array con MPI_Scatterv revisar la funcion hay algo mal que esta mandando fallo y hasta aqui ejecuta https://www.open-mpi.org/doc/v1.4/man3/MPI_Scatter.3.php
     MPI_Scatter(data1, (last*N), MPI_INT, partdata1, (last*N), MPI_INT, 0, MPI_COMM_WORLD);
     //printf("linea 105, rank %d\n", rank);
     MPI_Scatter(data2, (last*N), MPI_INT, partdata2, (last*N), MPI_INT, 0, MPI_COMM_WORLD);
     //printf("linea 107, rank %d\n", rank);

   //medición del tiempo de comunicación
    gettimeofday(&tv2, NULL); //segunda medicion
    int microseconds1 = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);//calculo de tiempo
   //Impresión tiempo de computación
    printf("1. Time procces %d = %lf\n", rank, (double) microseconds1/1E6);
    
    //Conteo de distancia
        gettimeofday(&tv3, NULL);//medimos tiempo al inicio
        //printf("linea 117, rank %d\n", rank);
    if (rank!=(size-1)) {    
        for (i = 0; i < bloque; i++) {
        //printf("linea 120, rank %d\n", rank);    
            for (j = 0; j < N; j++) {
                result[i] += base_distance(partdata1[i * N + j], partdata2[i * N + j]);                             
            }  
            //printf("posicion %d  valor %d rank %d\n",i, result[i] ,rank);  
        }
      } else{
      	   
      	   for (i = 0; i < last; i++) {
           //printf("linea 128, rank %d\n", rank);    
            for (j = 0; j < N; j++) {
                result[i] += base_distance(partdata1[i * N + j], partdata2[i * N + j]);  
                }
               // printf("posicion %d  valor %d rank %d\n",i, result[i], rank); 
          }
      }
        //printf("linea 134, rank %dn", rank);
        //medición del tiempo de computación
        gettimeofday(&tv4, NULL);
        int microseconds2 = (tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
        //Impresión tiempo de computación
        printf("2.Time procces %d = %lf\n", rank, (double) microseconds2/1E6);   

//medición del tiempo de comunicación
    gettimeofday(&tv5, NULL);
    //Funcion que une los vectores resultado en uno solo https://www.open-mpi.org/doc/v1.4/man3/MPI_Gather.3.php
    //revisar si debemos limitar el gather a los procesos< size -1 y realizar uno aparte para el ultimo 
    MPI_Gather(result, bloque, MPI_INT, totalresult, bloque, MPI_INT, 0, MPI_COMM_WORLD);

    //medición del tiempo de comunicación
    gettimeofday(&tv6, NULL);

    int microseconds3 = (tv6.tv_usec - tv5.tv_usec)+ 1000000 * (tv6.tv_sec - tv5.tv_sec);
   
        //Impresión tiempo de computación
        printf("3.Time procces %d = %lf\n", rank, (double) microseconds3/1E6);

    if(rank == 0) {
        /* Display result */
        if (DEBUG == 1) {
            int checksum = 0;
            for (i = 0; i < M; i++) {
                checksum += result[i];
            }
            printf("Checksum: %d\n ", checksum);
        } else if (DEBUG == 2) {
            for (i = 0; i < M; i++) {
                printf(" %d \t ", result[i]);
            }
        } else {
         int microseconds = (tv6.tv_usec - tv1.tv_usec)+ 1000000 * (tv6.tv_sec - tv1.tv_sec);
            printf("4.Time (seconds) = %lf\n", (double) microseconds / 1E6);
        }
    
    	for (int k = 0; k < M; k++){
    	   printf("Resultado posicion %d = %d\n", k, totalresult[k]);// imprimimos por pantalla
    	}
	free(data1); free(data2); free(totalresult); //liberamos el espacio de los elementos que sólo pertenence al proceso raiz, en este caso el 0
    }
    
    MPI_Finalize();	// se cierra la tarea dividida
     
    free(partdata1); free(partdata2); free(result);
    return 0;
}
