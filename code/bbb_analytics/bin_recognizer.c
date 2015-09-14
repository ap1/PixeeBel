#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define NMAX 8
#define SAMPLE_T 0.0001 // assuming 10 KHz sampling
#define __DEB                    {printf("Reached %s (%d)\n",__FILE__,__LINE__);}

// -- random number generator, 0 <= RND < 1 -- //
#define RND(p) ((*(p) = (*(p) * 7141 + 54773) % 259200) * (1.0 / 259200.0))

// -- external calls -- //
void cdft(int, int, double *);                      // fft library calls
void rdft(int, int, double *);                      // fft library calls

// -- function prototypes -- //
void put_values(double* arr, int n, double val);    // set a constant value to array
void put_zeros(double* arr, int n);                 // zero out an array
void print_array(double* arr, int n);               // printf double array
void generate_array(double* arr, int n);            // n-random values
void to_complex(double* arr, int n_real);           // replace v0,v1,v2,v3... with v0,0,v1,0,v2,0...
void complex_magnitude(double *arr, int n_real);      // get n_real real abs values from 2*n_real complex values
void fftfreq(double* arr, int n, float d);
void analyze_bin(double* arr, double* freqs, int n);

int main()
{
    // generate random 256-point data
    double *arr, *freqs;

    // allocate array
    arr     = (double*)malloc(NMAX * 2 * sizeof(double));
    freqs   = (double*)malloc(NMAX     * sizeof(double));

    generate_array(arr, NMAX);

    // put_zeros(arr, NMAX);
    // arr[0] = 1.0;

    //put_values(arr, NMAX, 1.0);
    
    print_array(arr, NMAX);

    to_complex(arr, NMAX);
    print_array(arr, NMAX *  2);

    cdft(NMAX * 2, -1, arr); // -1 for some reason matches python/numpy
    print_array(arr, NMAX * 2);

    complex_magnitude(arr, NMAX);
    print_array(arr, NMAX);

    fftfreq(freqs, NMAX, SAMPLE_T);
    print_array(freqs, NMAX);

    free(arr);
    return 0;
}

void put_values(double* arr, int n, double val)
{
    int j;
    for (j = 0; j < n; j++)
    {
        arr[j] = val;
    }      
}

void put_zeros(double* arr, int n)
{
    put_values(arr, n, 0.0);
}

void print_array(double* arr, int n)
{
    int j;
    for (j = 0; j < n; j++)
    {
        printf("% 0.2f ", arr[j]);
    }
    printf("\n");
}

void generate_array(double* arr, int n)
{
    int j, seed = 0;
    for (j = 0; j < n; j++)
    {
        arr[j] = RND(&seed);
    }
}

void to_complex(double* arr, int n)
{
    int j;
    for (j = (n-1); j >= 0; j--)
    {
        arr[j * 2 + 1] = 0.0;
        arr[j * 2 + 0] = arr[j];
    }
}

void complex_magnitude(double *arr, int n_real)
{
    int j;
    for (j = 0; j < n_real; j++)
    {
        double r = arr[2 * j];
        double i = arr[2 * j + 1];
        arr[j] = sqrt(r*r + i*i);
    }
}

void fftfreq(double* arr, int n, float d)
{
    int j;
    double scale = 1.0 / (d * (double)n);
    if(n % 2 == 0)
    {
        for (j = 0; j < n; j++)
        {
            if(j < (n/2))           arr[j] = scale * (double)(j);
            else                    arr[j] = scale * (double)(j - n);
        }
    }
    else
    {
        for (j = 0; j < n; j++)
        {
            if(j <= ((n-1)/2))      arr[j] = scale * (double)(j);
            else                    arr[j] = scale * (double)(j - n);
        }
    }
}