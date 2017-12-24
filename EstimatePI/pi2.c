#include <stdio.h>
#include <math.h>
#include <mpi.h>

int MPI_FattreeColectiva(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
	int numprocs, rank;
	MPI_Status status;

	MPI_Comm_size(comm, &numprocs);
	MPI_Comm_rank(comm, &rank);

	int i=0;
	int TAG = 0;
	if (rank == root){ //hago dos bucles, uno desde cero al root-1 y otro desde root+1 al final xq a el mismo no se envia
		for (i = root+1; i < numprocs; ++i)
			MPI_Send(buf,count, datatype,i, TAG, comm);
		for (i = 0; i < root -1; ++i)
			MPI_Send(buf,count, datatype,i, TAG, comm);
	}
	else
		MPI_Recv(buf,count, datatype,root,0, comm, &status);	
	return 0;
}


int MPI_BinomialColectiva ( void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
	int numprocs, rank;
	MPI_Status status;
	MPI_Comm_size(comm, &numprocs);
	MPI_Comm_rank(comm, &rank);

	int i;
	int norecibido = rank;
	int TAG = 0;
	//al principio el root que es el 0 es el unico para el cual norecibido es falso, por lo tanto es el unico que 
	//no entra en el segundo if. 
	//Como solo se tiene que entrar en el segundo if una vez, cuando se entra, norecibido se pone a cero y no se vuelve 
	//a entrar.
	for ( i = 0; i < log2(numprocs); ++i){
		if ((rank <= pow(2,i)-1)&&(rank+pow(2,i)<numprocs)){
			//la segunda condicion es para que no envie a procesos que no existen
			printf("proceso -> %d envia a proceso -> %d\n",rank,(int)(rank+pow(2,i)));
			MPI_Send(buf,count, datatype,rank+pow(2,i), TAG, comm);
		}		
		if (norecibido ){
			MPI_Recv(buf,count, datatype,MPI_ANY_SOURCE,TAG, comm, &status);	
			norecibido=0;		
		}
	}
	return 0;
}

int MPI_Reducee(void *buff, void *recvbuff, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm){
	int j;
	int TAG = 1;
	double temp = 0;
	int numprocs, rank;

	MPI_Status status;
	MPI_Comm_size(comm, &numprocs);
	MPI_Comm_rank(comm, &rank);

	if (rank == root)	{
			temp = *(double*) buff; //temp se inicializa a su buff

			for (j = 0; j<root;j++){ //de 0 a root-1
				MPI_Recv(buff,1, datatype,MPI_ANY_SOURCE,TAG,MPI_COMM_WORLD, &status);
				temp += *(double*) buff; //de cada proceso menos de si mismo, recibe los buffs y los suma a temp
			}	

			for (j = root+1; j<numprocs;j++){//de root+1 a numprocs
				MPI_Recv(buff,1, datatype,MPI_ANY_SOURCE,TAG,MPI_COMM_WORLD, &status);
				temp += *(double*) buff; 
			}	

		*(double*)recvbuff = temp;
	}
		else
			MPI_Send(buff,1,datatype,root,TAG, MPI_COMM_WORLD);
	return 0;
}



int main(int argc, char *argv[]){

    int i = 1, done = 0;
    double PI25DT = 3.141592653589793238462643;
    double pi, h, sum, x,suma;
	int numprocs, rank,n;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (!done){

		if (rank == 0)	{
		    printf("\nEnter the number of intervals: (0 quits) \n");
		    scanf("%d",&n);
		}

		MPI_FattreeColectiva(&n,1,MPI_INT,0,MPI_COMM_WORLD);
		//MPI_BinomialColectiva(&n,1,MPI_INT,0,MPI_COMM_WORLD);

		    if (n == 0) 
				break;

		    h   = 1.0 / (double) n;
		    sum = 0.0;
		    for (i = rank+1; i <= n; i+=numprocs) {
		        x = h * ((double)i - 0.5);
		        sum += 4.0 / (1.0 + x*x);
		    }
		    pi = h * sum;

		MPI_Reducee(&pi,&suma,1, MPI_DOUBLE, MPI_SUM,0, MPI_COMM_WORLD);
		if (rank == 0){
			printf("pi is approximately %.16f, Error is %.16f\n", suma, fabs(suma - PI25DT));
		}
	}
		MPI_Finalize();
	return 0;
}

