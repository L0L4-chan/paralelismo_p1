#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

//DeclaraciÃ³n de las funciones colectivas. Se pasan los parametros para enviar y recibir mensajes
int MPI_BinomialColectiva (void * buff, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
int MPI_FlattreeColectiva (void * buff, void * recvbuff, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);


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
    int size, rank;
    int n, count = 0;
    char *cadena;
    char L;

    //InicializaciÃ³n del entorno MPI
    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// tomamos el valor del numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);// tomamos el valor del rango de cada proceso


    //P0 es el que envÃ­a la informaciÃ³n a los otros procesos. Inicializavos las variables en proceso 0
    if(rank==0){
        n = atoi(argv[1]);//tamaÃ±o de la cadena
        L = *argv[2];//Letra a contar

    }
    //esta funcion realiza el send y el recieved de las variables n y L
    MPI_BinomialColectiva(&n,1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_BinomialColectiva(&L,1, MPI_CHAR, 0, MPI_COMM_WORLD );

    // se realiza la inicializacion de la cadena
    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);  //inicializamos la cadena


    //operacion de conteo recorriendo la cadena
    for(int i=rank; i<n; i= i +size){
        if(cadena[i] == L){
            count++;
        }
    }
    //esta funcion realiza el send y el recieved del contador de cada proceso. Se envÃ­a la info desde el resto de procesos a P0
    MPI_FlattreeColectiva(&count,&count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //Print del resultado por P0
    if(rank==0){
        printf("El numero de apariciones de la letra %c es %d\n", L, count);
    }

    // se cierra la tarea dividida
    MPI_Finalize();
    free(cadena);
    exit(0);
}

//FunciÃ³n de envÃ­o de n y L
int MPI_BinomialColectiva(void * buff, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {

    //Declaramos e inicializamos el nÃºmero de procesos y el rango dentro de la funciÃ³n
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //Todos los procesos excepto el cero se quedan a la espera para recibir la informaciÃ³n de las variables n y L
    if (rank != 0) {
        MPI_Recv(buff, count, datatype, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);//recibe
    }

    //Bucle para envÃ­o de argumentos.
    for (int j = 0; j < size; j++) {

        int n = pow(2, j);
        if (rank + n < size && rank < n) {

            //envÃ­o del dato en cuestion (n o L segÃºn diga la funciÃ³n)
            MPI_Send(buff, count, datatype, (rank + n), 0, comm);
        }
    }
    return 0;
}

//FunciÃ³n de envÃ­o y recepciÃ³n del contador
int MPI_FlattreeColectiva (void * buff, void * recvbuff, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){ //mantenemos cabecera

    //Declaramos e inicializamos el nÃºmero de procesos y el rango dentro de la funciÃ³n
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int aux = 0;

    //ComprobaciÃ³n de errores
    if (op != MPI_SUM)
        return MPI_ERR_OP;
    else if (datatype != MPI_INT)
        return MPI_ERR_TYPE;
    else if (count != 1)
        return MPI_ERR_COUNT;

    //Si es proceso raÃ­z, recibe el contador de cada proceso y lo suma
    if(rank==root) {
        aux = *(int*)recvbuff;
        for (int j = 1; j < size; j++) {
            MPI_Recv(recvbuff, count, datatype, j, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);
            aux = aux + *(int*) recvbuff;
        }
        *(int*) recvbuff = aux;
    }
    else{
        //EnvÃ­o de cada contador a P0 para sumarlo.
        MPI_Send(buff,1,datatype, 0,0, comm );
    }
    return 0;
}


