#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#pragma css task input(size, part) \
        output(A[size],B[size],C[size],D[size])
void funkcja1(float* A, float* B, float* C, float* D, int size, int part)
{
 int i;
 for (i = 0; i < size ; ++i)
 {
   A[i] = 1.0 * (i + part*size);
   B[i] = 1.0;
   C[i] = 0.0;
   D[i] = 0.0;
 }
}

#pragma css task input(size) \
        input(A[size]) \
        inout(B[size]) \
        output(C[size])
void funkcja2(float* A, float* B, float* C, int size)
{
 int i;
 for (i = 0; i < size ; ++i)
 {
   C[i] = A[i] - B[i];
   B[i] = B[i] + 1.0;
 }
}

#pragma css task input(size) \
        input(B[size], C[size]) \
        output(D[size])
void funkcja3(float* B, float* C, float* D, int size)
{
 int i,j;
 for (i = 0, j=size-1; i < size ; ++i , --j)
   D[i] = C[j] + B[i];
}


int main()
{

 int size = 256;
 int i=0,j=0;
 float *A , *B, *C, *D;

 A = (float*) malloc( size *sizeof(float));
 B = (float*) malloc( size *sizeof(float));
 C = (float*) malloc( size *sizeof(float));
 D = (float*) malloc( size *sizeof(float));

 /* SEKWENCYJNIE
 for (i = 0; i < size; i++)
 {
   A[i] = 1.0 * i;
   B[i] = 1.0;
   C[i] = 0.0;
   D[i] = 0.0;
 }

 for (i = 0; i < size ; ++i)
 {
   C[i] = A[i] - B[i];
   B[i] = B[i] + 1.0;
 }

 for (i = 0, j=size-1; i < size ; ++i , --j)
   D[i] = C[j] + B[i];
 */

 /* ROWNOLEGLE */

 #pragma css start

 for (i=0; i<4; ++i)
   funkcja1(&A[i*size/4],&B[i*size/4],&C[i*size/4],&D[i*size/4], size/4, i);

 #pragma css wait on (A, B)

 for (i=0; i<4; ++i)
   funkcja2(&A[i*size/4],&B[i*size/4],&C[i*size/4], size/4);

 #pragma css wait on (B, C)

 for (i=0; i<4; ++i)
   funkcja3(&B[i*size/4],&C[ (3-i)*size/4],&D[i*size/4], size/4);

 #pragma css wait on (D)

 #pragma css finish

 for (i = 0; i < size; i++)
   printf("%.0f ",D[i]);

 printf("startujemy");
 return 0;
}
