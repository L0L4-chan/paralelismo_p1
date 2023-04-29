#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define DEBUG 1

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
    //DeclaraciÃ³n de variables
    int i, j, bloque;
    int *partdata1, *partdata2;
    int *result;
    struct timeval  tv1, tv2, tv3, tv4; //Variables para contabilizar los tiempos de comunicaciÃ³n y computaciÃ³n
    int size, rank;
    int *data1, *data2;//el proceso 0 inicializa las matrices completas y el array completo
    int *totalresult;// las declaramos aqui porque si no nos da fallo al compilar

    //InicializaciÃ³n del entorno MPI
    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// tomamos el valor del numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);// tomamos el valor del rango de cada proceso

    /* Initialize Matrices */
    //Solo el proceso cero inicializa las matrices y el vector resultado
    if(rank == 0) {
        //reserva de memoria para las matrices y vector resultado solo con el proceso 0
        data1 = (int *) malloc(M*N*sizeof(int));
        data2 = (int *) malloc(M*N*sizeof(int));
        totalresult = (int *) malloc(M*sizeof(int));
        //InicializciÃ³n matrices
        for (i = 0; i < M; i++) {
            for (j = 0; j < N; j++) {
                /* random with 20% gap proportion */
                data1[i * N + j] = fast_rand();
                data2[i * N + j] = fast_rand();
            }
        }
    }

    if (M%size >0){
        bloque = (M/size)+1; //calculamos el tamaÃ±o del bloque en filas que se pasa a cada proceso
    }
    else{
        bloque = M/size; //calculamos el tamaÃ±o del bloque en filas que se pasa a cada proceso
    }


    //cada proceso realizan la reserva de espacio de la matriz bloque y el vector de resultados
    partdata1 = (int *) malloc(bloque*N*sizeof(int));
    partdata2 = (int *) malloc(bloque*N*sizeof(int));
    result = (int *) malloc(bloque*sizeof(int));

    //inicio de la comunicacion
    gettimeofday(&tv1, NULL);

    //EnvÃ­o de los array con MPI_Scatter https://www.open-mpi.org/doc/v1.4/man3/MPI_Scatter.3.php
    MPI_Scatter(data1, bloque*N, MPI_INT, partdata1, bloque*N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(data2, bloque*N, MPI_INT, partdata2, bloque*N, MPI_INT, 0, MPI_COMM_WORLD);

    //final de la comunicaciÃ³n y comienzo de la computaciÃ³n
    gettimeofday(&tv2, NULL);

    //Conteo de distancias en cada bloque
    for (i = 0; i < bloque; i++) {//for filas
        for (j = 0; j < N; j++) {//for columnas
            result[i] += base_distance(partdata1[i * N + j], partdata2[i * N + j]);
        }
    }

    //mediciÃ³n del tiempo final del computaciÃ³n e inicio de la comuniciaciÃ³n
    gettimeofday(&tv3, NULL);

    //Funcion que une los vectores resultado en uno solo https://www.open-mpi.org/doc/v1.4/man3/MPI_Gather.3.php
    MPI_Gather(result, bloque, MPI_INT, totalresult, bloque, MPI_INT, 0, MPI_COMM_WORLD);

    //mediciÃ³n del tiempo final de comunicaciÃ³n
    gettimeofday(&tv4, NULL);

    //CÃ¡lculo de tiempos de comunicaciÃ³n y computaciÃ³n
    int microseconds1 = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);
    int microseconds2 = (tv3.tv_usec - tv2.tv_usec)+ 1000000 * (tv3.tv_sec - tv2.tv_sec);
    int microseconds3 = (tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);


    //ImpresiÃ³n tiempo de comunicacion y computaciÃ³n

    printf("Time computation %d = %lf\n", rank, (double) microseconds2/1E6);
    printf("Time comunication %d = %lf\n", rank, (double) (microseconds3+microseconds1)/1E6);

    //ImpresiÃ³n de resultados. Solo el rango 0 suma los resultados para calcular el total
    if(rank == 0) {
        /* Display result */
        if (DEBUG == 1) {
            int checksum = 0;
            for (i = 0; i < M; i++) {
                checksum += totalresult[i];
            }
            printf("Checksum: %d\n ", checksum);
        } else if (DEBUG == 2) {
            for (i = 0; i < M; i++) {
                printf(" %d \t ", totalresult[i]);
            }
        } else {
            int microseconds = (tv4.tv_usec - tv1.tv_usec)+ 1000000 * (tv4.tv_sec - tv1.tv_sec);
            printf("Time (seconds) = %lf\n", (double) microseconds / 1E6);
        }
        //liberamos el espacio de los elementos que sÃ³lo pertenence al proceso raiz, en este caso el 0
        free(data1); free(data2); free(totalresult);
    }

    //Se finaliza el entorno MPI y se liberan las reservas de espacio
    MPI_Finalize();	// se cierra la tarea dividida
    free(partdata1); free(partdata2); free(result);
    return 0;
}
