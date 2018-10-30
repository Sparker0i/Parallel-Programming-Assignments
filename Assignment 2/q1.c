#include <stdio.h>
#include <mpich/mpi.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 30
#define MAX_NUM 1000

typedef struct
{
    int * array;
    int index;
} Bucket;

int compare( const void * n1, const void * n2)
{
    return (*(int*)n1 - *(int*)n2);
}

void bucket_insert(Bucket * b, int x)
{
    b->array[b->index] = x;
    b->index = b->index +1;
}

void slave(int rank , int size)
{
    int i;
    int num_buckets = size;

    int large_bucket_size = ceil(ARRAY_SIZE/(float) size);
    int small_bucket_size = large_bucket_size;

    int *array;

    /* Creating n random numbers */
    if (rank == 0)
    {
        array = (int *) malloc(sizeof(int) * ARRAY_SIZE);

        srand(time(NULL));

        for (i = 0; i < ARRAY_SIZE; ++i)
        {
            array[i] = rand() % MAX_NUM;
            printf("Array [%d] = %d\n" , i , array[i]);
        }
    }

    /* Initializing Buckets */
    Bucket **buckets = (Bucket **) malloc(sizeof(Bucket *) * num_buckets);
    for (i = 0; i < num_buckets; ++i) 
    {
        buckets[i] = (Bucket *) malloc(sizeof(Bucket));
        buckets[i] -> array = (int *) malloc(sizeof(int) * small_bucket_size * 2.0);
        buckets[i] -> index = 0;
    }

    Bucket large_bucket;

    int *my_bucket_array = (int *) malloc(sizeof(int) * large_bucket_size * 4.0);
    large_bucket.array = my_bucket_array;
    large_bucket.index = 0;
    /* End Initialization */

    int range_min = (MAX_NUM * rank) / size;
    int range_max = (MAX_NUM * (rank + 1)) / size;
    printf("Bucket %d, Range = (%d , %d)\n" , rank , range_min , range_max);

    MPI_Scatter(array , large_bucket_size , MPI_INT , large_bucket.array ,large_bucket_size , MPI_INT , 0 , MPI_COMM_WORLD);
    int dest;

    for (i = 0; i < large_bucket_size; ++i)
    {
        dest = (large_bucket.array[i] * num_buckets)/MAX_NUM;
        if (dest == rank)
        {
            bucket_insert(&large_bucket , large_bucket.array[i]);
        }
        else
        {
            bucket_insert(buckets[dest],large_bucket.array[i]);
        }
    }
    MPI_Request* requests = (MPI_Request *) malloc(sizeof(MPI_Request) * size);
    for (i = 0; i < size; ++i)
    {
        if (i != rank)
        {
            MPI_Isend(buckets[i] -> array , small_bucket_size * 2 , MPI_INT , i , buckets[i] -> index , MPI_COMM_WORLD , &requests[i]);
        }
    }
    MPI_Status status;
    int current = large_bucket.index;
    
    for (i = 0; i < size - 1; ++i)
    {
        MPI_Recv(&large_bucket.array[current] , small_bucket_size * 2 , MPI_INT , MPI_ANY_SOURCE , MPI_ANY_TAG , MPI_COMM_WORLD , &status);
        current += status.MPI_TAG;
    }
    large_bucket.index = current;
    qsort(&large_bucket.array[0] , current , sizeof(int) , compare);

    int *sizes = (int *) malloc(sizeof(int) * size);
    MPI_Gather(&current , 1 , MPI_INT , sizes , 1 , MPI_INT , 0 , MPI_COMM_WORLD);
    
    int *disp = (int *) malloc(sizeof(int) * size);
    if (rank == 0)
    {
        disp[0] = 0;
        for (i = 1; i < size; ++i)
        {
            disp[i] = disp[i - 1] + sizes[i - 1];
        }

        for (i = 0; i < size; ++i)
        {
            range_min = disp[i];
            range_max = disp[i] + sizes[i];
            printf("Rank [%d] [%d] => [%d] Disp = %d Size = %d\n",i,range_min,range_max, disp[i],sizes[i]);
        }
    }
    MPI_Gatherv(large_bucket.array , current , MPI_INT , array , sizes , disp , MPI_INT , 0 , MPI_COMM_WORLD);

    if(rank == 0)
    {
        printf("Array size: %d\n" , ARRAY_SIZE);
        for(i = 0; i < ARRAY_SIZE; i++)
            printf("array[%d] = %d\n" , i , array[i]);
        free(array);
    }
    return;
}

int main(int argc , char **argv)
{
    int rank , size;

    MPI_Init(&argc , &argv);
    MPI_Comm_rank(MPI_COMM_WORLD , &rank);
    MPI_Comm_size(MPI_COMM_WORLD , &size);

    if (ARRAY_SIZE % size != 0)
    {
        printf("ARRAY SIZE and Number of Processes do not divide equally");
        exit(0);
    }

    slave(rank , size);
    MPI_Finalize();
    return 0;
}