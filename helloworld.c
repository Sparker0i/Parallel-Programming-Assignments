#include "mpich/mpi.h"
#include "stdio.h"

int main(int argc , char *argv)
{
    MPI_Init(NULL , NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD , &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD , &world_rank);

    int n;
    if (world_rank == 0)
    {
        scanf("%d",  &n);
        MPI_Send(&n , 1 , MPI_INT , 1 , 0 , MPI_COMM_WORLD);
    }
    else
    {
        MPI_Recv(&n , 1 , MPI_INT , 0 , 0 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
        printf("Process with Rank %d received number %d from Process 0\n" , world_rank , n);
    }

    MPI_Finalize();
    return 0;
}