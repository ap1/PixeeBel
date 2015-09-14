#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>

#include "msg_queue.h"
#include "pb_msg.h"
#include "pb_bins.h"
#include "bin_reader.h"

#define NMAX (SAMPLES_PER_MSG)
#define SAMPLE_T 0.0001 // assuming 10 KHz sampling
#define __DEB                    {printf("Reached %s (%d)\n",__FILE__,__LINE__);}
#define MAXBINS 64

double channels[NR_CHANNELS][NMAX * 2];
double running_sum[NR_CHANNELS];
double running_sqr_sum[NR_CHANNELS];
double running_count;

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
void complex_magnitude(double *arr, int n_real);    // get n_real real abs values from 2*n_real complex values
void fftfreq(double* arr, int n, float d);
void analyze_bin(double* mags, double* freqs, int n, bin* bins, int n_bins);
void analyze_msg(msg* m, double* freqs, bin* bins, int nbins);
void initialize_running_sums();

int main()
{
    // generate random 256-point data
    double *arr, *freqs;
    bin bins[MAXBINS];
    int nbins;
    int qid;
    msg m;

    qid = mg_open( "/tmp/pb_msgqueue" );

    printf("qid %d\n", qid);

    printf( "Draining pending data\n" );
    mg_drain( qid, MG_TYPE );
    printf( "   done\n" );

    // allocate array
    arr     = (double*)malloc(NMAX * 2 * sizeof(double));
    freqs   = (double*)malloc(NMAX     * sizeof(double));

    //generate_array(arr, NMAX);

    // put_zeros(arr, NMAX);
    // arr[0] = 1.0;

    //put_values(arr, NMAX, 1.0);
    
    // // print_array(arr, NMAX);

    initialize_running_sums();

    fftfreq(freqs, NMAX, SAMPLE_T);

    nbins = bins_load("../bbb_common/bin_defs.txt", bins);

    while((mg_recv( qid, MG_TYPE, &m ) > 0))
    {
        analyze_msg(&m, freqs, bins, nbins);
    }

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
    if(n % 2 == 0) {
        for (j = 0; j < n; j++) {  if(j < (n/2))        arr[j] = scale * (double)(j);
                                   else                 arr[j] = scale * (double)(j - n);
        }
    } else {
        for (j = 0; j < n; j++) {  if(j <= ((n-1)/2))   arr[j] = scale * (double)(j);
                                   else                 arr[j] = scale * (double)(j - n);
        }
    }
}

void analyze_bin(double* mags, double* freqs, int n, bin* bins, int n_bins)
{
    // -- for the bin, find if peak exists and energy in the bin -- //
    int j, k, n_valid_freqs;
    double *bin_energies;
    double *bin_counts;

    bin_energies = (double*) malloc(n_bins * sizeof(double));
    bin_counts = (double*) malloc(n_bins * sizeof(double));

    n_valid_freqs = ((n % 2) == 0) ? (n/2) : (1 + (n-1)/2);

    for(k = 0; k < n_bins; k++)
    {
        bin_energies[k] = 0.0;
        bin_counts[k] = 0.0;
    }

    for(j=0; j<n_valid_freqs; j++)
    {
        for(k = 0; k < n_bins; k++)
        {
            if(
                freqs[j] >= ((double)bins[k].freq_low) && 
                freqs[j] <= ((double)bins[k].freq_high))
            {
                bin_energies[k]  += (mags[j] * mags[j]);
                bin_counts[k]    += 1.0;
            }
        }
    }

    for(k = 0; k < n_bins; k++)
    {
        if(bin_counts[k] > 0.0)
        {
            bin_energies[k] = sqrt(bin_energies[k] * (1.0 / bin_counts[k]));
        }
        else
        {
            bin_energies[k] = 0.0;
        }
        //printf("bin %d to %d energy is %f\n", bins[k].freq_low, bins[k].freq_high, bin_energies[k]);
        printf(" %0.2f", bin_energies[k]);
    }
    printf(" | ");
    

    free(bin_energies);
    free(bin_counts);
}

void to_double(SAMPLE *samples, double *doubles, int n)
{
    int j;

    for(j=0; j<n; j++)
    {
        doubles[j] = (double)samples[j];
    }
}

void remove_dc_component(double* data, int n, double meanvalue, double stdev)
{
    int j;
    for(j=0; j<n; j++)
    {
        data[j] = ((data[j] - meanvalue) / stdev);
    }
}

void initialize_running_sums()
{
    int j;
    for(j = 0; j < NR_CHANNELS; j++)
    {
        running_sum[j] = 0.0;
        running_sqr_sum[j] = 0.0;
    }
    running_count = 0.0;
}

void update_running_sums(double* avgs, double* stdevs)
{
    int c,j;

    for(j = 0; j < NMAX; j++)
    {
        for(c = 0; c < NR_CHANNELS; c++)
        {
            running_sum[c]      += channels[c][j];
            running_sqr_sum[c]  += (channels[c][j] * channels[c][j]);
        }
        running_count += 1.0;
    }

    for(c = 0; c < NR_CHANNELS; c++)
    {
        avgs[c] = (running_sum[c] / running_count);
        stdevs[c] = sqrt((running_sqr_sum[c] / running_count) - avgs[c] * avgs[c]);
    }

    if(running_count > 100000) initialize_running_sums();
}

void analyze_msg(msg* m, double* freqs, bin* bins, int nbins)
{
    int chId;
    double avgs[4];
    double stdevs[4];
    double* curChannel;
    //printf ("-------\n");
    printf("analyzing %d bins: ", nbins);
    for (chId = 0; chId < NR_CHANNELS; ++chId)
    {
        to_double(m->data[chId], channels[chId], NMAX);
    }
    update_running_sums(avgs, stdevs);
    for (chId = 0; chId < NR_CHANNELS; ++chId)
    {
        
        //printf("%0.2f, %0.2f | ", avgs[chId], stdevs[chId]);
        
        //print_array(channels[chId], 16);
        remove_dc_component(channels[chId], NMAX, avgs[chId], stdevs[chId]);

        //print_array(channels[chId], 16);

        to_complex(channels[chId], NMAX);
        //print_array(channels[chId], 16);
        cdft(NMAX * 2, -1, channels[chId]); // -1 for some reason matches python/numpy
        complex_magnitude(channels[chId], NMAX);
        analyze_bin(channels[chId], freqs, NMAX, bins, nbins);
    }
    printf("\n");
}