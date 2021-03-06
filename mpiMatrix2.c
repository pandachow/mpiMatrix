#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "mpi.h" 

#include "tdarray.h"

#define MAX_DIMENSION 1024
#define TAG_MSG   0
#define TAG_MNT   3
#define TAG_AI    4
#define TAG_BI    5
#define TAG_CI    6

extern int errno;
int m, n; /* The arguments */
int t; /* 't' stores the number of rows or columns processed by each process*/
int rank[MAX_DIMENSION];
float** marray[MAX_DIMENSION];
float** A; /* Pointer to Matrix A */
float** B; /* Pointer to Matrix B */
float** C; /* Pointer to Matrix C */
float** TMP;
FILE* fa;
FILE* fb;

int displayMatrix(float** p, const char * matrixName, int row, int col) {
    
    int i, j;
    
    printf("--------- %s ---------\n", matrixName);
    for(i=0; i<row; i++) {
        for(j=0; j<col; j++) {
            printf("%8.2f", p[i][j]);
        }
        printf("\n");
    }
}

/* calculate production of two matrices */
int mmult(float** ma, float** mb, float** mc, int arow, int acol) {
    
    int i, j, k;
    int from, to;
    from = 0;
    to = acol;
    if (to>n) to = n;
    
    for (i=0; i<arow; i++) {
        for (j=0; j<arow; j++) {
            for(k=from; k<to; k++){
                printf("calculation funtion: A[%i][%i], B[%i][%i]\n" , i, j, j, k);
                mc[i][j] += ma[i][k]*mb[k][j];
            }
        }
    }
}

/* calculate addition of two matrices */
int madd(float** ma, float** mb, int row, int col) {
    
    int i, j;
    
    for (i=0; i<row; i++) {
        for (j=0; j<col; j++) {
            ma[i][j] += mb[i][j];
        }
    }
}

int main(int argc, char * args[]) {
    
    int i, j, k;
    float a, b;
    int ci, cj;
    int myrank;
    time_t cur = 0;
    struct timeval beginTime, beginTime2, beginTime3, endTime;
    MPI_Status status; 
    float message[20];
    float * data;
    float * point;
    int count;
    
    /* get the time before parallel computing */
//    gettimeofday(&currentTime, NULL);
//    printf("Begin: \tsecond:%lu\tusec:%lu\n", currentTime.tv_sec, currentTime.tv_usec);
    
    /* parallel part begins here */
    MPI_Init( &argc, &args ); 
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank ); 
    MPI_Comm_size(MPI_COMM_WORLD, &m); 
    
    /* Calculate C=A*B using multithread. */
    if(myrank==0) {
//        printf("myrank = %d\n", myrank);
    
        /* deal with the arguments */
        if(argc <2) {
            printf("\nUsage: mpirun -np m mpiMatrix1 n\n\tn: the dimension of the matrices, n > m \n\tm: the number of threads, m > 0\n\n");
            MPI_Finalize(); 
            exit(0);
        }
        
        n = strtol(args[1], 0, 0);
        if(n<=0 || m>n ) {
            perror("The dimension input is not illegal. The correct value is 0<m<n. \n");
            MPI_Finalize(); 
            exit(0);
        }
        
        /* calculate the column size for each process */
        t = n/m;
        if ((t*m)<n) t++;
        printf("n=%d, m=%d\n", n, m);
        
        /* rank=0, this process do the addition */
        createTdArray(sizeof(float), n, n, &A);
        createTdArray(sizeof(float), n, n, &B);
        createTdArray(sizeof(float), n, n, &C);
        createTdArray(sizeof(float), n, n, &TMP);
        
        /* initialize the matrix A & B with random floating-point numbers */
        time(&cur);
        //seed = (unsigned)cur;
        // arguments for array arrangement
        char seps[] = " ";
        char *token;
        int arr[3];

        //arguments for read line by line
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fa = fopen("a", "r");
        fb = fopen("b", "r");
        gettimeofday(&beginTime, NULL);
        while((read = getline(&line, &len, fa)) != -1){
            int x = 0;
            token = strtok(line, seps);
            while(token != NULL){
                arr[x++] = atoi(token);
                token = strtok(NULL, seps);
            }
            A[arr[0]][arr[1]] = arr[2];
        }

        while((read = getline(&line, &len, fb)) != -1){
            int x = 0;
            token = strtok(line, seps);
            while(token != NULL){
                arr[x++] = atoi(token);
                token = strtok(NULL, seps);
            }
            B[arr[0]][arr[1]] = arr[2];
        }
        fclose(fa);
        fclose(fb);
        puts("first");
        displayMatrix(A, "A", n, n);
        displayMatrix(B, "B", n, n);
        /*
        for(i=0; i<n; i++) {
            for(j=0; j<n; j++) {
                a = (float)rand_r(&seed);
                b = (float)rand_r(&seed);
                A[i][j] = a/b;
                a = (float)rand_r(&seed);
                b = (float)rand_r(&seed);
                
                B[i][j] = a/b;
                a = (float)random();
                b = (float)random();
                A[i][j] = a/b;
                a = (float)random();
                b = (float)random();
                B[i][j] = a/b;
            }
        } /* end of initialization of matrix A & B */
        gettimeofday(&beginTime2, NULL);
        message[0]=m;
        message[1]=n;
        message[2]=t;
        printf("begin sending TAG_MNT.\n");
        for(i=1; i<m; i++) {
            MPI_Send(message, 20, MPI_FLOAT, i, TAG_MNT, MPI_COMM_WORLD);
        }
        printf("TAG_MNT sent.\n");
        /* Send parts of A to other corresponding processes */
        printf("beging sending matrix A\n");
        for (i=0; i<n; i++) {
            for (j=1; j<m; j++) {
                MPI_Send(&(A[i][j*t]), (n-j*t)<t?(n-j*t):t, MPI_FLOAT, j, TAG_AI, MPI_COMM_WORLD);
            }
        }
        printf("matrix A sent\n");
        /* Send parts of B to other corresponding processes */
        printf("begin sending matrix B\n");
        for (i=1; i<m; i++) {
            count = ( (n-i*t)<t?(n-i*t):t )*n;
            MPI_Send(B[i*t], count, MPI_FLOAT, i, TAG_BI, MPI_COMM_WORLD);
        }
        printf("matrix B sent.\n");
        
        /* calculate C0 */
        printf("beging rank %i calculation.\n", myrank);
        mmult(A, B, TMP, n, t);
        printf("end rank %i calculation.", myrank);
        //MPI_Barrier(MPI_COMM_WORLD);
//        for(i=1; i<m; i++) {
//            MPI_Recv(TMP[0], n*n, MPI_FLOAT, MPI_ANY_SOURCE, TAG_CI, MPI_COMM_WORLD, &status);
//            MPI_Get_count(&status, MPI_FLOAT, &count);
//            if(count==n*n)
//                madd(C, TMP, n, n);
//        }
        
        MPI_Reduce(TMP[0], C[0], n*n, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        
        FILE *result = fopen("result", "w");
        gettimeofday(&beginTime3, NULL);
        for(i = 0; i<n; i++){
            for(j = 0; j<n; j++)
                fprintf(result, "%f ", C[i][j]);
            fprintf(result, "\n");
        }
        fclose(result);

        gettimeofday(&endTime, NULL);
        printf("Output Microseconds:%lu\n", (endTime.tv_sec-beginTime3.tv_sec)*1000000+endTime.tv_usec-beginTime3.tv_usec);
        printf("Input Microseconds:%lu\n", (beginTime2.tv_sec-beginTime.tv_sec)*1000000+beginTime2.tv_usec-beginTime.tv_usec);
        printf("Communication & Calculation Microseconds:%lu\n", (beginTime3.tv_sec-beginTime2.tv_sec)*1000000+beginTime3.tv_usec-beginTime2.tv_usec);
        /*
        if(n<=10) {
            displayMatrix(A, "A", n, n);
            displayMatrix(B, "B", n, n);
            displayMatrix(C, "C", n, n);
        }
        */
        displayMatrix(C, "C", n, n);
    } /* end of 'rank==0' */
    else { /* process of which rank is greater than 0 */
        /* rank>0, do the multiplication */
        MPI_Recv(message, 20, MPI_FLOAT, 0, TAG_MNT, MPI_COMM_WORLD, &status);
        m = (int)message[0];
        n = (int)message[1];
        t = (int)message[2];
        if(myrank==m-1) t=(n-myrank*t)<t?(n-myrank*t):t;
        //!printf("m=%d, n=%d, t=%d\n", m, n,t);
        
        createTdArray(sizeof(float), t, n, &A);
        createTdArray(sizeof(float), n, t, &B);
        createTdArray(sizeof(float), n, n, &C);
        createTdArray(sizeof(float), n, n, &TMP);
        
        /* receive parts of A from P0 */
        for (i=0; i<n; i++) {
            MPI_Recv(&A[i][0], t, MPI_FLOAT, 0, TAG_AI, MPI_COMM_WORLD, &status);
        }
        
        /* receive parts of B from P0 */
        MPI_Recv(B[0], t*n, MPI_FLOAT, 0, TAG_BI, MPI_COMM_WORLD, &status);
        
        /* calculate Cj */
        mmult(A, B, TMP, n, t);
        
        /* send the result back to P0 */
        //MPI_Barrier(MPI_COMM_WORLD);
        MPI_Reduce(TMP[0], C[0], n*n, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize(); 
}