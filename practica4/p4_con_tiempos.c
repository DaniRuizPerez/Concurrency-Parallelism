#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>
#include "task-creation.h"

#define DEBUG 0
#define N 1024
#define DIE_TAG 666666666


void insertion_sort( double *v, int n ) {
  int i,j;

  for( i = 0; i < n; ++i ) {
    double insert = v[i];
    for( j = i-1; (j>=0) && (v[j]>insert); --j ) 
      v[j+1] = v[j];
    v[j+1] = insert;
  }
}    

void inicialice(double *vector, double *matriz){
	/* Initialize matriz and Vector */
	struct timeval tv1;
	int i,j;
	gettimeofday(&tv1, NULL );
	srand(tv1.tv_usec );
	for(i=0;i<N;i++) {
		vector[i] = (double)rand()/RAND_MAX;
		for(j=0;j<N;j++) 
			matriz[N*i+j] = i*j*((double)rand()/RAND_MAX);
	}
}

double* calculos (int rows, double *matriz_local, double *vector){
	int i,j;
	double *resultado = malloc(sizeof(double)*rows);

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

	//ejemplo, N = 16, numprocs = 7
	/*
	rows[0] -> 3
	rows[1] -> 3
	rows[2] -> 3
	rows[3] -> 3
	rows[4] -> 3
	rows[5] -> 1
	rows[6] -> 0
	*/

}

int main(int argc, char *argv[] ) {

	int numprocs;
	int rank;
    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (numprocs < 2){
		printf("\nINDIQUE UN NUMERO DE PROCESOS VÁLIDO (>= 2)\n\n");
		MPI_Finalize();
		return 0;	
	}

	int i = 0;
    int *rows = malloc(sizeof(int)*numprocs);
	double *matriz = NULL;
	double *matriz_local;
	double *vector = malloc(sizeof(double)*N);
	double *result_parcial;
	double *result = NULL;
    struct timeval  tv1, tv2, tv3, tv4,tiempototal_sec_ini, tiempototal_sec_fin;
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
			result = malloc(sizeof(double)*N);
			matriz = malloc(sizeof(double)*N*N);
			inicialice(vector, matriz);
		}

	gettimeofday(&tv4, NULL);
    tcomp =(tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);

	result_parcial = malloc(sizeof(double)*rows[rank]);
	//Declaro matriz local, cada proceso tiene una
	matriz_local = malloc(sizeof(double)*rows[rank]*N);
			
	gettimeofday(&tv1, NULL);
		//distribuyo el vector
		MPI_Bcast(vector,N , MPI_DOUBLE, 0, MPI_COMM_WORLD );
		// Scatter de los datos de la matriz
		MPI_Scatterv(matriz, num_elements,display,MPI_DOUBLE,matriz_local,num_elements[rank],MPI_DOUBLE,0, MPI_COMM_WORLD);
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
	MPI_Gatherv(result_parcial, num_elements[rank], MPI_DOUBLE,result, num_elements, display,MPI_DOUBLE,0, MPI_COMM_WORLD);
	gettimeofday(&tv2, NULL);
    tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);






	double *copia_result;
	double ttotal_sec;

    if (rank == 0){  
	   	copia_result = malloc(sizeof(double)*N);
	   	int k = 0;
	   	for (k = 0; k < N; ++k)
	   		copia_result[k] = result[k];
	}
	MPI_Barrier(MPI_COMM_WORLD);

    tcomm = 0;
    tcomp = 0;

	int task_n ;
	MPI_Status status;
	slice_t *slice = NULL;
	double *result_aux = malloc(sizeof(double)*MAX_TASK_SIZE);
	int count = 0;
	double final = 0;
	int j = 0;
	int envie_die_tag = 0;
		/*
		1 -> Calculo las slices
		2 -> envio a todos los procesos el cacho inicial. En este y en todos los sends, el tag es
		la direccion de comienzo de ese cacho, para que luego cuando el exclavo haga el send con
		el vector ordenado, devuelva el tag de donde empieza para dar la direccion de memoria del comienzo
		3 -> Entro en el bucle:
			3.1 -> Hago el probe para saber el tamaño y la direccion de comienzo del vector
			3.2 -> recibo el vector
			3.3 -> Si ya no tengo mas cosas que enviar, envio el die tag a todos los procesos para que lo 
			reciban y se mueran cuando puedan
			3.4 -> recibo los trozos de vector que me quedan por recibir y me muero
		*/	

    if (rank == 0){  
    	i = 1; 	
		slice = (slice_t *)task_creation(result,N, &task_n);
		printf("(PERF) task_n -> %d\n",task_n);
		while(i < numprocs && i <= task_n){
			//printf("(PERF) ROOT: envio al -> %d\n", i);
			gettimeofday(&tv1, NULL);
				MPI_Send(&result[slice[i-1].slice_init], slice[i-1].slice_size,MPI_DOUBLE,i,slice[i-1].slice_init, MPI_COMM_WORLD);
			gettimeofday(&tv2, NULL);
    		tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);
			i++;
		}
		i--; //hago esto porque empiezo a contar en el 1
		while(1){
			gettimeofday(&tv1, NULL);
				MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			gettimeofday(&tv2, NULL);
    		tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

    		gettimeofday(&tv1, NULL);
				MPI_Get_count(&status, MPI_DOUBLE, &count);
				MPI_Recv(&result[status.MPI_TAG],count,MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			gettimeofday(&tv2, NULL);
    		tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

			if (!envie_die_tag){
				if (i >= task_n){	
					for (j = 1; j < numprocs; ++j){
						gettimeofday(&tv1, NULL);
							MPI_Send(&final, 1,MPI_DOUBLE,j,DIE_TAG, MPI_COMM_WORLD);	
						gettimeofday(&tv2, NULL);
    					tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);						
					}
					envie_die_tag = 1;
					//el caso de numprocs == 2 es especial
					if (i == task_n && numprocs == 2){
						gettimeofday(&tv3, NULL);
							insertion_sort(result, N);
						gettimeofday(&tv4, NULL);
    					tcomp +=(tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
						break;
					}
				}
				else {
					//printf("(PERF) ROOT: envio al -> %d\n", status.MPI_SOURCE);
					gettimeofday(&tv1, NULL);
						MPI_Send(&result[slice[i-1].slice_init], slice[i-1].slice_size,MPI_DOUBLE,status.MPI_SOURCE,slice[i-1].slice_init, MPI_COMM_WORLD);
					gettimeofday(&tv2, NULL);
    				tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);
					i++;
				}
			}
			else 
				//si i-numprocs (numero de iteracioes de este bucle) es el numero de tareas, tengo que salir
				if (i == task_n){
					gettimeofday(&tv3, NULL);
							insertion_sort(result, N);
					gettimeofday(&tv4, NULL);
    				tcomp +=(tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
					break;
				}
		}
	}

	else{
		while (1){		
			gettimeofday(&tv1, NULL);
				MPI_Probe(0,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			gettimeofday(&tv2, NULL);
    		tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

			MPI_Get_count(&status, MPI_DOUBLE, &count);
			if (status.MPI_TAG != DIE_TAG){
				gettimeofday(&tv1, NULL);
					MPI_Recv(result_aux,count,MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				gettimeofday(&tv2, NULL);
    			tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

					gettimeofday(&tv3, NULL);
						insertion_sort(result_aux,count);
					gettimeofday(&tv4, NULL);
    				tcomp +=(tv4.tv_usec - tv3.tv_usec)+ 1000000 * (tv4.tv_sec - tv3.tv_sec);
				//envio de vuelta la direccion de comienzo
				gettimeofday(&tv1, NULL);
					MPI_Send(result_aux,count,MPI_DOUBLE,0,status.MPI_TAG, MPI_COMM_WORLD);
				gettimeofday(&tv2, NULL);
    			tcomm +=(tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

			}
			else {
				break;
			}
		}
	}

	
    //medir los tiempos y tal
    double tcommF, tcompF;
    MPI_Reduce(&tcomm, &tcommF, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&tcomp, &tcompF, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    tcompF /= numprocs;
    tcommF /= numprocs;


    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0){  
		gettimeofday(&tiempototal_sec_ini, NULL);
			insertion_sort(copia_result, N);
		gettimeofday(&tiempototal_sec_fin, NULL);
    	ttotal_sec =(tiempototal_sec_fin.tv_usec - tiempototal_sec_ini.tv_usec)+ 1000000 * (tiempototal_sec_fin.tv_sec - tiempototal_sec_ini.tv_sec);
    }


    //IMPRIMIR POR PANTALLA
	if (rank == 0){
		printf("\n\n\n");
		if (DEBUG)
			for (i = 1; i < N; ++i){
				printf("%f\n",result[i]);
				if( result[i] < result[i-1]) {
			      	printf( "ERROR: result not ordered\n" );
			      	MPI_Finalize();
			      	exit( 0 );
				}
			}
		printf("\n\n\n");
		printf ("(PERF) Time_Computacion (seconds) = %lf\n", (double) tcompF/1E6);
		printf ("(PERF) Time_comunicacion  (seconds) = %lf\n", (double) tcommF/1E6);
		printf ("(PERF) TIEMPO TOTAL ORDENACION (seconds) = %lf\n", (double) (tcompF+tcommF)/1E6);
		printf ("(PERF) TIEMPO TOTAL ORDENACION SECUENCIAL (seconds) = %lf\n", (double) ttotal_sec/1E6);
		printf ("(PERF) MFlops = %f\n", (2*N*N/(tcommF+tcompF)));	
	}

	MPI_Finalize();
	return 0;
}
