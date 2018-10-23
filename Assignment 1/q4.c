#include "mpich/mpi.h"
#include "stdio.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

float calc_avg(int *p , int s)
{
	float sum = 0;
	for(int i = 0; i < s; i++){
		sum += *(p + i);
	}
	return (float) (sum / s);
}

int main(int argc , char *argv)
{
    MPI_Init(NULL , NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD , &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD , &world_rank);

	srand(time(NULL) + world_rank);
	int *rbuf , *sender;
	if (world_rank == 0)
	{
		rbuf = (int *) malloc(world_size * 1 * sizeof(int));
	}
	/* Otherwise it is sender, even if it is rank = 0 */
	sender = (int *) malloc(1 * sizeof(int));
	sender[0] = rand() % 100;
	
	MPI_Gather(sender , 1 , MPI_INT , rbuf , 1 , MPI_INT , 0 , MPI_COMM_WORLD);
	float avg;
	if (world_rank == 0)
	{
		for (int i = 0; i < world_size; ++i)
		{
			printf("%d," , rbuf[i]);
		}
		avg = calc_avg(rbuf , world_size);
		printf("\n%f\n" , avg);
	}
	MPI_Bcast(&avg , 1 , MPI_FLOAT , 0 , MPI_COMM_WORLD);
	
	float temp_sdi = pow(abs(sender[0] - avg) , 2);
	float *rcvbuf = (float *) malloc(world_size * 1 * sizeof(float));
	MPI_Gather(&temp_sdi , 1 , MPI_INT , rcvbuf , 1 , MPI_INT , 0 , MPI_COMM_WORLD);

	if (world_rank == 0)
	{
		float sum = 0;
		for (int i = 0; i < world_size; ++i)
		{
			sum += rcvbuf[i];
		}
		sum /= ((float) world_size);
		printf("%f\n" , sqrt(sum));
	}
    MPI_Finalize();
    return 0;
}