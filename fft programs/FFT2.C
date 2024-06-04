/* 2-D fft2 program */
/* nn is the dimension of the 2-D array..assuming square.        */
/* The input is read by row ordering and stored in "data" in complex form. */
/* For example, if your input is 
        1  2
        3  4
   That is, nn = 2, data will have dimension [0..2*nn*nn-1], 
   which is [0..7]. You have to store the input as
   data[0] = real part of the (0,0)th element,
   data[1] = imaginary part of the (0,0)th element.
   Thus, we have data[0] = 1.0, data[1] = 0.0,
   data[2] = 2.0, data[3] = 0.0, data[4] = 3.0, data[5] = 0.0,
   data[6] = 4.0, data[7] = 0.0.                               */

/* Note that the data in this program is zero-offset, that is
   the range of data is [0..2*nn*nn-1]. So, you can invoke fft2.c
   by, for example, fft2(data,256,1).                             */

/* isign = 1 means 2-D discrete Fourier transform   */
/* isign = -1 means 2-D inverse Fourier transform.  */
/* When you are using inverse fft, there is no need to divide
   the results by (nn*nn), since it will be done in the program.   */

#include <stdio.h>
#include <math.h>
//#include "fft1.c"

void fft2(float data[],int nn,int isign)
//float data[];
//int nn,isign;

{
   int i,j,ii,jj,k,m;
   //void fft1();
   void fft1(float data[],int nn,int isign);
   float dd[512];

   m = nn << 1;
   
   /* read data by row ordering */
   for (i=0;i<nn;++i) {
     k = i << 1;
     for (j=0;j<nn;++j) {
        jj = j << 1;
        ii = m*j;
        dd[jj] = data[ii+k];
        dd[jj+1] = data[ii+1+k];
     }
	 
     fft1(dd-1,nn,isign);
 
     for (j=0;j<nn;++j) {
        jj = j << 1;
        ii = m*j;
        data[ii+k] = dd[jj];
        data[ii+1+k] = dd[jj+1];
     }
   }    /* end of i loop */

   for (i=0;i<nn;++i) {
     for (j=0;j<nn;++j) {
       jj = j << 1;
       ii = m*i;
       dd[jj] = data[ii+jj];
       dd[jj+1] = data[ii+jj+1];
     }
	 
     fft1(dd-1,nn,isign);
     for (j=0;j<nn;++j) {
       jj = j << 1;
       ii = m*i;
       data[ii+jj] = dd[jj];
       data[ii+jj+1] = dd[jj+1];
     }
   }    /* end of i loop */

 }  /* end of main */
/* ---------------------------------------------------------- */

