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
    float avg=0.0;
	int sd;
	int arr[world_size];
    if (world_rank == 0)
    {
		for (int i = 0; i < world_size; i++)
        {
			arr[i] = rand() % 100;
			printf("%d " , arr[i]);
		}
		avg = calc_avg(arr , world_size);
        printf("\nAverage is %f\n",avg);
	}
	MPI_Bcast(&avg , 1 , MPI_FLOAT , 0 , MPI_COMM_WORLD);
	float rcv = pow(abs(arr[world_rank] - avg) , 2);
	float *sub_avg = NULL;
	float ans = 0.0;
	if (world_rank == 0)
		sub_avg = malloc(sizeof(float) * world_size);
	MPI_Gather(&rcv , 1 , MPI_FLOAT , sub_avg , 1 , MPI_FLOAT , 0 , MPI_COMM_WORLD);
	if (world_rank == 0)
    {
		for (int i = 0;i < world_size; i++) {
			printf("%f\n" , *(sub_avg + i));
			ans += *(sub_avg + i);
		}
		sd = (int) (ans/world_size);
		sd = sqrt(sd);
		printf("SD = %d\n", sd);
	}
    MPI_Finalize();
    return 0;
}