#include <stdio.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]){
	int numprocs, rank;

    int i = 1,k,j, done = 0, n;
    double PI25DT = 3.141592653589793238462643;
    double pi, h, sum, x;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    while (!done){

	if (rank == numprocs-1)	{
        printf("Enter the number of intervals: (0 quits) \n");
        scanf("%d",&n);
        if (n == 0) 
			break;
		for (k = 0; k<numprocs-1;k++)
			MPI_Send(&n,1,MPI_INT,k,0, MPI_COMM_WORLD);
	}
	else
		MPI_Recv(&n,1, MPI_INT,numprocs-1,0,MPI_COMM_WORLD, &status);

//ahora calculamos
        h   = 1.0 / (double) n;
        sum = 0.0;
        for (i = rank+1; i <= n; i+=numprocs) {
            x = h * ((double)i - 0.5);
            sum += 4.0 / (1.0 + x*x);
        }
        pi = h * sum;
		double pi2 = pi; 

	if (rank == numprocs-1)	{
		double suma = 0;
		for (j = 1; j<numprocs;j++){
			MPI_Recv(&pi2,1, MPI_DOUBLE,MPI_ANY_SOURCE,1,MPI_COMM_WORLD, &status);
			suma+=pi2;
		}
		pi+=suma;
        printf("pi is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
	}
	else
		MPI_Send(&pi2,1,MPI_DOUBLE,numprocs-1,1, MPI_COMM_WORLD);
    }
MPI_Finalize();
}

