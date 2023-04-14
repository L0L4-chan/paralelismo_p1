#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>


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

int main(int argc, char *argv[])
{
    if(argc != 3){
        printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
        exit(1);
    }



    int n, size,rank, count = 0;
    char *cadena;
    char L;

    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//devuelve el rango de cada proceso


    if(rank==0){
	    // leemos los datos, solo proceso 0
        n = atoi(argv[1]);//tamaño de la cadena
        L = *argv[2];//Letra a contar

        for(int j = 1 ; j<size; j++){//bucle para envio de argumentos

            MPI_Send(&n,1,MPI_INT, j, 0, MPI_COMM_WORLD );//enviamos el tamaño de la cadena
            MPI_Send(&L,1, MPI_CHAR, j, 0, MPI_COMM_WORLD );//enviamos el char
       
        }
    }
    else{
        MPI_Recv(&n,1,MPI_INT, 0,MPI_ANY_TAG, MPI_COMM_WORLD,MPI_STATUS_IGNORE);//cada proceso recibe n y L
        MPI_Recv(&L,1,MPI_CHAR, 0,MPI_ANY_TAG, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
       
    }

    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);  //inicializamos la cadena
	
    //operacion de conteo
    
    for(int i=rank; i<n; i= i +size){
        if(cadena[i] == L){
       
            count++;
        }
    }

    if(rank==0){ //el proceso 0 es el encargado de sumar e imprimir
        int aux;
        for(int j = 1 ; j<size; j++){//recepcion del contador

            MPI_Recv(&aux,1,MPI_INT, j,MPI_ANY_TAG, MPI_COMM_WORLD,MPI_STATUS_IGNORE);//recibimos de cada particion
            count = count + aux; //suma de los contadores	
        }
       	
        
        printf("El numero de apariciones de la letra %c es %d\n", L, count);

    }
    else{//el resto de procesos envia sus resultados
        MPI_Send(&count,1,MPI_INT, 0,0, MPI_COMM_WORLD );//cada proceso (menos el 0) envia su contador
    }

    MPI_Finalize();	// se cierra la tarea dividida
    free(cadena);
    exit(0);
}


