#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

//InicializaciÃ³n de la cadena a tratar en el problema.
void inicializaCadena(char *cadena, int n){
    int i;
    for(i=0; i<n/2; i++){
        cadena[i] = 'A';
    }
    for(i=n/2; i<3*n/4; i++){
        cadena[i] = 'C';
    }
    for(i=3*n/4; i<9*n/10; i++){
        cadena[i] = 'G';
    }
    for(i=9*n/10; i<n; i++){
        cadena[i] = 'T';
    }
}

int main(int argc, char *argv[]){
    //ComprobaciÃ³n de que se pasen los parametros suficientes para la ejecuciÃ³n del programa
    if(argc != 3){
        printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaÃ±o de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
        exit(1);
    }

    //DeclaraciÃ³n de variables
    int n, size,rank, count = 0;
    char *cadena;
    char L;

    //InicializaciÃ³n del entorno MPI
    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//devuelve el rango de cada proceso

    //P0 es el que envÃ­a la informaciÃ³n a los otros procesos. Inicializavos las variables en proceso 0
    if(rank==0){
        n = atoi(argv[1]);//tamaÃ±o de la cadena
        L = *argv[2];//Letra a contar
    }

    //esta funcion realiza el send y el recieved de las variables n y L
    MPI_Bcast(&n,1, MPI_INT, 0, MPI_COMM_WORLD );//enviamos el tamaÃ±o de la cadena
    MPI_Bcast(&L,1, MPI_CHAR, 0, MPI_COMM_WORLD );//enviamos el char

    // se realiza la inicializacion de la cadena
    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);  //inicializamos la cadena

    //operacion de conteo
    for(int i=rank; i<n; i= i +size){
        if(cadena[i] == L){
            count++;
        }
    }

    //esta funcion realiza el send y el recieved del contador de cada proceso. Se envÃ­a la info desde el resto de procesos a P0
    int aux;
    MPI_Reduce(&count,&aux, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //Print del resultado por P0
    if(rank==0){
        printf("El numero de apariciones de la letra %c es %d\n", L, aux);
    }


    MPI_Finalize();	// se cierra la tarea dividida
    free(cadena);
    exit(0);
}
