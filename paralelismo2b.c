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
    //se realiza la suma de los datos
    MPI_FlattreeColectiva(&count,&count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if(rank==0){    
    printf("El numero de apariciones de la letra %c es %d\n", L, count);
    }
  

    MPI_Finalize();	// se cierra la tarea dividida
    free(cadena);
    exit(0);
}


int MPI_BinomialColectiva(void * buff, int count, MPI_Datatype datatype, int root, MPI_Comm comm) {//mismos parametros

    if (rank != 0) { //todos los procesos menos el 0 recibiran sólo una vez
                MPI_Recv(buff, count, datatype, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);//recibe
    }
   //todos los procesos 
    for (int j = 0; j < size; j++) {//bucle para envio de argumentos

        int n = pow(2, j);
        if (rank + n < size && rank < n) { //si cumplen las condiciones 

            MPI_Send(buff, count, datatype, (rank + n), 0, comm);     //envian el dato en cuestion
        }
    }
return 0;
}




int MPI_FlattreeColectiva (void * buff, void * recvbuff, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){ //mantenemos cabecera
    int aux = 0;
// gestion de excepciones posibles 
    if (op != MPI_SUM)//operacion que no sea suma
        return MPI_ERR_OP;
    else if (datatype != MPI_INT)//tipo de dato que no sea entero
        return MPI_ERR_TYPE;
    else if (count != 1)//que se este enviando más de un dato
        return MPI_ERR_COUNT;

    //printf("count %d, rank, %d\n", count, rank);
    
	if(rank==root) { //se recepciona y suman los contadores
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
    else{//cada proceso envia su contador
        MPI_Send(buff,1,datatype, 0,0, comm );

        //printf("count %d, rank, %d\n", c, rank);
	}
  return 0;
 }
  

