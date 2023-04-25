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

    int i, j, k;
    int *partdata1, *partdata2;
    int *result;
    struct timeval  tv1, tv2, tv3, tv4,tv5,tv6;
    int size, rank;
    int *data1, *data2;//el proceso 0 inicializa las matrices completas y el array completo
    int *totalresult;// las declaramos aqui porque si no nos da fallo al compilar
    
    //Inicialización del entorno MPI
    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// tomamos el valor del numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);// tomamos el valor del rango de cada proceso   
    
    /* Initialize Matrices */
    if(rank == 0) { //el proceso cero inicializa las matrices y el vector resultado
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
    int last = M-(bloque * size);// calculamos el resto
    //todos los procesos realizan la reserva de espacio de la matriz bloque y el vector de resultados
    partdata1 = (int *) malloc(bloque*N*sizeof(int));
    partdata2 = (int *) malloc(bloque*N*sizeof(int));
    result = (int *) malloc(bloque*sizeof(int));
    //medición comienzo de la comunicacion comunicación
    gettimeofday(&tv1, NULL);//medimos tiempo y almacenamos
    //Envío de los array con MPI_Scatter https://www.open-mpi.org/doc/v1.4/man3/MPI_Scatter.3.php
     MPI_Scatter(data1, bloque*N, MPI_INT, partdata1, bloque*N, MPI_INT, 0, MPI_COMM_WORLD);
     MPI_Scatter(data2, bloque*N, MPI_INT, partdata2, bloque*N, MPI_INT, 0, MPI_COMM_WORLD);
   //medición del tiempo de comunicación 1 y 2
    gettimeofday(&tv2, NULL); 
    int microseconds1 = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);//calculo de tiempo
   //Impresión tiempo de comunicacion
    //printf("Time procces %d = %lf\n", rank, (double) microseconds1/1E6);
    //medición comienzo del calculo
    //Conteo de distancia
    gettimeofday(&tv3, NULL);//medimos tiempo al inicio
   
    for (i = 0; i < bloque; i++) {//for filas
          for (j = 0; j < N; j++) {//for columnas
              result[i] += base_distance(partdata1[i * N + j], partdata2[i * N + j]);                             
            }  
        }
      if (last!= 0 && rank==0){//si existe resto el proceso cero lo calcula
      	   for (i = bloque*size; i < M; i++) {//desde la ultima fila no enviad   
            for (j = 0; j < N; j++) {//for columnas  y almacena en el vector totalresult
                totalresult[i] += base_distance(data1[i * N + j], data2[i * N + j]);  
                }
          }
      }
        //medición del tiempo final del calculo
        gettimeofday(&tv4, NULL);
        int microseconds2 = (tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
        //Impresión tiempo de computación
        printf("Time computation %d = %lf\n", rank, (double) microseconds2/1E6);   

//medición del tiempo de comunicación
    gettimeofday(&tv5, NULL);
    //Funcion que une los vectores resultado en uno solo https://www.open-mpi.org/doc/v1.4/man3/MPI_Gather.3.php
    MPI_Gather(result, bloque, MPI_INT, totalresult, bloque, MPI_INT, 0, MPI_COMM_WORLD);
    //medición del tiempo de comunicación
    gettimeofday(&tv6, NULL);
    int microseconds3 = (tv6.tv_usec - tv5.tv_usec)+ 1000000 * (tv6.tv_sec - tv5.tv_sec);
    //Impresión tiempo de comunicacion
    printf("Time comunication %d = %lf\n", rank, (double) (microseconds3+microseconds1)/1E6);

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
            printf("Time (seconds) = %lf\n", (double) microseconds / 1E6);
        }
    
    	for (k = 0; k < M; k++){
    	   printf("Position %d, result %d\n", k, totalresult[k]);// imprimimos por pantalla
    	}
	free(data1); free(data2); free(totalresult); //liberamos el espacio de los elementos que sólo pertenence al proceso raiz, en este caso el 0
    }  
    MPI_Finalize();	// se cierra la tarea dividida   
    free(partdata1); free(partdata2); free(result);
    return 0;
}
