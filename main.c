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

    //int i, n, count=0;
    
    int n, count = 0;
    char *cadena;
    char L;

    n = atoi(argv[1]);//numero de procesos
    L = *argv[2];//Letra a contar

    cadena = (char *) malloc(n*sizeof(char));
    //inicializaCadena(cadena, n);
    
    //Pendiente creacion de los argumentos 
    
    MPI_Init(&argc, & argv);//inicializamos los procesos
    MPI_Comm_size(MPI_COMM_WORLD, &argv[3]);// 
    MPI_Comm_rank(MPI_COMM_WORLD, &t_id);///asignamos un identificador
    
/*
    for(i=0; i<n; i++){
        if(cadena[i] == L){
            count++;
        }
    }

    
    printf("El numero de apariciones de la letra %c es %d\n", L, count);
    
*/
    int rank  = Get_rank();
 
    if(rank == 0){
    inicializaCadena(cadena, n);// el proceso 0 debe iniciar el array
    //enviar a los otros procesos los indices a comprobar 
        // realizar la tarea
    //Recibir de los otros procesos los resultados o el aviso de finalizacion de estado 
    printf("El numero de apariciones de la letra %c es %d\n", L, count);// e imprimir la salida
	}else{
        //si no es el proceso 0 debera realizar su tarea
        //recibir sus indices
        //realizar tarea
        //mandar respuesta
	
	}
	
    MPI_Finalize();	// se cierra la tarea dividida
    free(cadena);
    exit(0);
}
