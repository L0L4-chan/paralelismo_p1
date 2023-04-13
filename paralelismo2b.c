#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>


 int MPI_BinomialColectiva (void * buff, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
 int MPI_FlattreeColectiva (void * buff, void * recvbuff, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);
 
 int size, rank;
 
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



    int n, count = 0;
    char *cadena;
    char L;

    MPI_Init(&argc, &argv);//inicializamos que los procesos se comuniquen
    MPI_Comm_size(MPI_COMM_WORLD, &size);// numero de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//devuelve el rango de cada proceso

    

    if(rank==0){
        n = atoi(argv[1]);//tamaño de la cadena
        L = *argv[2];//Letra a contar

     }
      //esta funcion realiza el send y el recieved
     MPI_BinomialColectiva(&n,1, MPI_INT, 0, MPI_COMM_WORLD );//enviamos el tamaño de la cadena
     MPI_BinomialColectiva(&L,1, MPI_CHAR, 0, MPI_COMM_WORLD );//enviamos el char
       
// se realiza la inicializacion de la cadena
    cadena = (char *) malloc(n*sizeof(char));
    inicializaCadena(cadena, n);  //inicializamos la cadena
	
   
 //operacion de conteo
    for(int i=rank; i<n; i= i +size){
        if(cadena[i] == L){
            count++;
        }
    }
    
    MPI_FlattreeColectiva(&count,&count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if(rank==0){    
    printf("El numero de apariciones de la letra %c es %d\n", L, count);
    }
  

    MPI_Finalize();	// se cierra la tarea dividida
    free(cadena);
    exit(0);
}


int MPI_BinomialColectiva(void * buff, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {

    if (rank != 0) {
                MPI_Recv(buff, count, datatype, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);//recibe
    }

    for (int j = 0; j < size; j++) {//bucle para envio de argumentos

        int n = pow(2, j);
        if (rank + n < size && rank < n) { 

            MPI_Send(buff, count, datatype, (rank + n), 0, comm);     //enviamos el dato en cuestion
        }
    }
return 0;
}




int MPI_FlattreeColectiva (void * buff, void * recvbuff, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){ //mantenemos cabecera
    int aux = 0;

    if (op != MPI_SUM)
        return MPI_ERR_OP;
    else if (datatype != MPI_INT)
        return MPI_ERR_TYPE;
    else if (count != 1)
        return MPI_ERR_COUNT;

    //printf("count %d, rank, %d\n", count, rank);
    
	if(rank==root) {
	aux = *(int*)recvbuff;
       // printf("El total es count %d y el rank es %d \n", aux, rank);
        for (int j = 1; j < size; j++) {
            MPI_Recv(recvbuff, count, datatype, j, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);
            aux = aux + *(int*) recvbuff;
            //printf("El total es count %d\n", aux);
        }
        //printf("El total es count %d\n", aux);
        
        *(int*) recvbuff = aux;

    }
    else{
        MPI_Send(buff,1,datatype, 0,0, comm );

        //printf("count %d, rank, %d\n", c, rank);
	}
  return 0;
 }
  

