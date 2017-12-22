#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>

#define DEBUG 1
#define N 1024

void inicialice(int *vector, int *matriz){
	int i,j;
	for(i=0;i<N;i++) {
		vector[i] = i;
		for(j=0;j<N;j++) {
			matriz[N*i+j] = i+j;	
		}
	}
}

int* calculos (int rows, int *matriz_local, int *vector){
	int i,j;
	int *resultado = malloc(sizeof(int)*rows);

	for (i = 0; i < rows; ++i){
		resultado[i]= 0;	
		for (j = 0; j < N; ++j)
			resultado[i] += vector[j] * matriz_local[N*i+j];
	}	
	return resultado;
}

int * inicializar_rows(int numprocs, int *rows){
	// Cálculo del número de filas por proceso

	//voy haciendo el bucle inicializando rows y compruebo que no se pase del rango y puedo modificarlo
	// en funcion del rango que queda
	//y cuando lo acabe, asigno al resto 0

	int i = 0, j = 0;
	while(i<numprocs){
		rows[i] = ceil( (float)N / (float)numprocs);
		if (N - rows[0]*(i+1) < 0){
			rows[i] = abs(N-rows[0]*(i));
			for (j = i+1; j < numprocs; ++j)
				rows[j] =0;
			break; 
		}
		i++;
	}
	return rows;

}

int main(int argc, char *argv[] ) {

	int numprocs;
	int rank;
    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int i = 0;
    int *rows = malloc(sizeof(int)*numprocs);
	int *matriz = NULL;
	int *matriz_local;
	int *vector = malloc(sizeof(int)*N);
	int *result_parcial;
	int *result = NULL;
    struct timeval  tv1, tv2, tv3, tv4;
    double tcomm, tcomp;
	int *num_elements = malloc(sizeof(int)*numprocs);
	int *display = malloc(sizeof(int)*numprocs);
	 
	gettimeofday(&tv3, NULL);
		//inicializo el array rows
		inicializar_rows(numprocs, rows);

		//calculo num_elements y display
		num_elements[0] = rows[0]*N;
		display[0] = 0;
		for (i = 1; i < numprocs; ++i){
			num_elements[i] = rows[i]*N;
			display[i] = display[i-1] + rows[i-1]*N;
		}

		/* Initialize matriz and Vector */
		if (rank == 0){
			result = malloc(sizeof(int)*N);
			matriz = malloc(sizeof(int)*N*N);
			inicialice(vector, matriz);
		}

	gettimeofday(&tv4, NULL);
    tcomp =(tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);

	result_parcial = malloc(sizeof(int)*rows[rank]);

	//Declaro matriz local, cada proceso tiene una
	matriz_local = malloc(sizeof(int)*rows[rank]*N);
			
	gettimeofday(&tv1, NULL);
		//distribuyo el vector
		MPI_Bcast(vector,N , MPI_INT, 0, MPI_COMM_WORLD );
		// Scatter de los datos de la matriz
		MPI_Scatterv(matriz, num_elements,display,MPI_INT,matriz_local,num_elements[rank],MPI_INT,0, MPI_COMM_WORLD);
	gettimeofday(&tv2, NULL);
    tcomm =(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

	//hago las multiplicaciones
    gettimeofday(&tv3, NULL);
    	//hago mis calculos
	    result_parcial = calculos(rows[rank], matriz_local,vector);

	    //calculos parael gatherv
		//Ahora num_elements y display estan en funcion de las columnas no de los elementos
			num_elements[0] = rows[0];
		display[0] = 0;
		for (i = 1; i < numprocs; ++i){
			num_elements[i] = rows[i];
			display[i] = display[i-1] + rows[i-1];
		}
	gettimeofday(&tv4, NULL);
    tcomp +=(tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);


	//Gather de todos los datos
	gettimeofday(&tv1, NULL);
	MPI_Gatherv(result_parcial, num_elements[rank], MPI_INT,result, num_elements, display,MPI_INT,0, MPI_COMM_WORLD);
	gettimeofday(&tv2, NULL);
    tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);


    //medir los tiempos y tal
    double tcommF, tcompF;
    MPI_Reduce(&tcomm, &tcommF, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&tcomp, &tcompF, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    tcompF /= numprocs;
    tcommF /= numprocs;



    //IMPRIMIR POR PANTALLA
	if (rank == 0){
		printf("\n\n\n");
		for (i = 0; i < N; ++i)
			printf("%d\n",result[i]);
		printf("\n\n\n");

		if (DEBUG){
			printf("\nCOSAS ANTES DEL SCATTERV\n");
			for(i=0;i<numprocs;i++)
				printf("numelements[%d]->%d\n",i,num_elements[i]);
			printf("\n");
			for(i=0;i<numprocs;i++)
				printf("display[%d]->%d\n",i,display[i]);

			if (rank == 0){
				printf("\n");
				for (i = 0; i < numprocs; ++i)
					printf("rows[%d] -> %d\n",i,rows[i]);
				printf("\n");
			}	
		}
		printf ("Time_Computacion (seconds) = %lf\n", (double) tcompF/1E6);
		printf ("Time_comunicacion (seconds) = %lf\n", (double) tcommF/1E6);
		printf ("MFlops = %d\n", (int)(2*N*N/(tcompF)));	
	}


	MPI_Finalize();
	return 0;
}

