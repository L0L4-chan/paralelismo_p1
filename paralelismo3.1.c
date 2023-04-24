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
    int *data1, *data2;
    int *partdata1, *partdata2;
    int *result, *totalresult;
    struct timeval  tv1, tv2, tv3, tv4,tv5, tv6;
    int size, rank;
    
    //Inicialización del entorno MPI
    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// tomamos el valor del numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);// tomamos el valor del rango de cada proceso

    int sendcounts[size];    // array  cuantos elementos van en cada proceso
    int displs[size];        // array donde comienza el segmento de cada particion
 
    int rem = (M)%size; // filas sobrantes de una division equitativa 
    int amount = (M/size)*N; //elementos por bloque
    int sum = 0;                // para calcular el inicio de cada segmento
 //rellenamos los arrays que contienen el numero de elementos y la posicion de inicio de cada bloque
    for (int i = 0; i < size; i++) {
        sendcounts[i] = amount;
        if (rem > 0) {
            sendcounts[i] = sendcounts[i] + N;
            rem --;
         if(rem == 1){amount = amount +N;} 
        }

        displs[i] = sum;
        sum += sendcounts[i];
    }

    result = (int *) malloc((sendcounts[rank]/N)*sizeof(int));// lo mismo para result va a ser del tamaño numero de filas y vamos a tener que crear en el cero uno nuevo para la recogida de datos,
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
     printf("linea 103, rank %d\n", rank);
    partdata1 = (int *) malloc(sendcounts[rank]*N*sizeof(int));
    partdata2 = (int *) malloc(sendcounts[rank]*N*sizeof(int));
     printf("linea 105, rank %d\n", rank);
//estamos realizando las mediciones de cada comunicacion por separado desde que se empieza a comunicar hasta que cada proceso recibe, tenemos la del calculo tambien por separado y luego recogemos el tiempo total, PREGUNTAR si el tiempo de comunicacion debe ser sumado
    //medición del tiempo de comunicación
    gettimeofday(&tv1, NULL);//medimos tiempo y almacenamos
     
    //Envío de los array con MPI_Scatterv revisar la funcion hay algo mal que esta mandando fallo y hasta aqui ejecuta https://www.open-mpi.org/doc/v1.4/man3/MPI_Scatterv.3.php
      MPI_Scatterv(data1, sendcounts, displs, MPI_INT, partdata1, amount, MPI_INT, 0, MPI_COMM_WORLD);
      
      //printf("linea 100, rank %d\n", rank);
      MPI_Scatterv(data2, sendcounts, displs, MPI_INT, partdata2, amount, MPI_INT, 0, MPI_COMM_WORLD);
      //printf("linea 102, rank %d\n", rank);

   //medición del tiempo de comunicación
    gettimeofday(&tv2, NULL); //segunda medicion
   int microseconds1 = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);//calculo de tiempo
        //Impresión tiempo de computación
        printf("1. Time procces %d = %lf\n", rank, (double) microseconds1/1E6);

    
    //Conteo de distancia
        gettimeofday(&tv3, NULL);//medimos tiempo al inicio
        //printf("linea 109, rank %d\n", rank);
       
        for (i = 0; i < (sendcounts[rank]/N); i++) {//al solo tener su trozo no es necesario el calculo 
        //printf("linea 117, rank %d\n", rank);    
            for (j = 0; j < N; j++) {
                result[i] += base_distance(partdata1[i * N + j], partdata2[i * N + j]);             
            }  
        }
        
        //printf("linea 132, rank %dn", rank);
        //medición del tiempo de computación
        gettimeofday(&tv4, NULL);
        int microseconds2 = (tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
        //Impresión tiempo de computación
        printf("2.Time procces %d = %lf\n", rank, (double) microseconds2/1E6);   

//medición del tiempo de comunicación
    gettimeofday(&tv5, NULL);
    //Funcion que une los vectores resultado en uno solo https://www.open-mpi.org/doc/v1.4/man3/MPI_Gatherv.3.php
    //revisar lo del vector resultado
    MPI_Gatherv(result, 1, MPI_INT, totalresult, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

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
	free(data1); free(data2); free(totalresult);
    }


    MPI_Finalize();	// se cierra la tarea dividida
    free(result);
    free(partdata1); free(partdata2); 
    return 0;
}
