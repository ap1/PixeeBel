#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdint.h>

#include "msg_queue.h"
#include "pb_msg.h"
#include "pb_bins.h"
#include "bin_reader.h"
#include "pb_visualize.h"

#define NMAX (SAMPLES_PER_MSG)
#define SAMPLE_T 0.0001 // assuming 10 KHz sampling
#define __DEB                    {printf("Reached %s (%d)\n",__FILE__,__LINE__);}
#define MAXBINS 64
#define NR_ACTIVE_CHANNELS 4

double channels[NR_ACTIVE_CHANNELS][NMAX * 2];


double running_sum[NR_ACTIVE_CHANNELS];
double running_sqr_sum[NR_ACTIVE_CHANNELS];
double running_count;


double *running_energy_sum;
double *running_energy_sqr_sum;
double running_energy_count;

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
void analyze_bin(double* mags, double* freqs, int n, bin* bins, int nbins, double* bin_energies);
void analyze_msg(msg* m, double* freqs, bin* bins, int nbins);
void initialize_running_sums();

// main: prepares structs and runs the main loop
// which receives all packets
int main()
{
    double *arr, *freqs;
    bin bins[MAXBINS];
    int nbins;
    int qid;
    int msg_count;
    double time_elapsed;
    struct timeval start_time, cur_time;
    msg m;

    printf("[init]\n");

    // -- setting up receiving queue -- //
    printf(" - opening message queue\n");
    qid = mg_open( "/tmp/pb_msgqueue" );
    printf(" - qid %d\n", qid);

    printf(" - draining old data\n");
    mg_drain( qid, MG_TYPE );
    printf(" - done\n" );

    // -- allocate arrays for data processing -- //
    printf(" - array allocations\n");
    freqs = (double*)malloc(NMAX * sizeof(double));

    printf(" - other initializations\n");
    initialize_running_sums();
    fftfreq(freqs, NMAX, SAMPLE_T);
    visualize_init();
    nbins = bins_load("../bbb_common/bin_defs.txt", bins);

    running_energy_sum      = (double*)malloc(nbins * sizeof(double));
    running_energy_sqr_sum  = (double*)malloc(nbins * sizeof(double));
    initialize_running_energies(nbins, running_energy_sum, running_energy_sqr_sum);

    printf("[main loop]\n");
    gettimeofday(&start_time, NULL);
    msg_count = 0;

    // -- main loop -- //
    while((mg_recv( qid, MG_TYPE, &m ) > 0))
    {
        analyze_msg(&m, freqs, bins, nbins);
        if(msg_count % 100 == 0 && msg_count >= 100)
        {
            gettimeofday(&cur_time, NULL);
            time_elapsed = 
                (double)(cur_time.tv_sec  - start_time.tv_sec) + 
                (double)(cur_time.tv_usec - start_time.tv_usec) * 0.000001;
            printf(" - current rate: %0.2f samples / sec\n", 256.0 * ((double)msg_count)/(double)time_elapsed);
        }
        msg_count++;
    }

    free(freqs);
    free(running_energy_sum);
    free(running_energy_sqr_sum);

    return 0;
}

// simple function to set all values in an array
// to a constant
void put_values(double* arr, int n, double val)
{
    int j;
    for (j = 0; j < n; j++)
    {
        arr[j] = val;
    }      
}

// simple function to zero out all values in an
// array
void put_zeros(double* arr, int n)
{
    put_values(arr, n, 0.0);
}

// simple function to print values in an array
void print_array(double* arr, int n)
{
    int j;
    for (j = 0; j < n; j++)
    {
        printf("% f ", arr[j]);
    }
    printf("\n");
}

// simple function to create an array of random
// values
void generate_array(double* arr, int n)
{
    int j, seed = 0;
    for (j = 0; j < n; j++)
    {
        arr[j] = RND(&seed);
    }
}

// function to convert real array (of size n) to complex
// by inserting interleaving zeroes
void to_complex(double* arr, int n)
{
    int j;
    for (j = (n-1); j >= 0; j--)
    {
        arr[j * 2 + 1] = 0.0;
        arr[j * 2 + 0] = arr[j];
    }
}

// function to compute magnitude of complex values
// in an array (n complex values, 2n doubles)
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

// compute list of frequencies for an n-point fft
// same as scipy/numpy version
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

// computes per-bin avg energy (as RMS value)
// given the frequency domain response of a signal
void analyze_bin(double* mags, double* freqs, int n, bin* bins, int nbins, double* bin_energies)
{
    // -- for the bin, find if peak exists and energy in the bin -- //
    int j, k, n_valid_freqs;
    double *bin_counts;

    bin_counts = (double*) malloc(nbins * sizeof(double));

    n_valid_freqs = ((n % 2) == 0) ? (n/2) : (1 + (n-1)/2);

    for(k = 0; k < nbins; k++)
    {
        bin_energies[k] = 0.0;
        bin_counts[k]   = 0.0;
    }

    for(j=0; j < n_valid_freqs; j++)
    {
        for(k = 0; k < nbins; k++)
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

    for(k = 0; k < nbins; k++)
    {
        if(bin_counts[k] > 0.0) bin_energies[k] = sqrt(bin_energies[k] * (1.0 / bin_counts[k]));
        else                    bin_energies[k] = 0.0;
    }
    free(bin_counts);
}

// converts samples to doubles
void to_double(SAMPLE *samples, double *doubles, int n)
{
    int j;

    for(j=0; j<n; j++)
    {
        doubles[j] = (double)samples[j];
    }
}

// display raw data for a channel (array of SAMPLEs)
void print_raw_channel(SAMPLE *samples, int n)
{
    int j;

    for(j=0; j<n; j++)
    {
        printf("%x ", samples[j]);
    }
    printf("\n");
}

// subtract constant value from a signal
// can also divide the resultant by another constant
void remove_dc_component(double* data, int n, double meanvalue, double stdev)
{
    int j;
    for(j=0; j<n; j++)
    {
        data[j] = ((data[j] - meanvalue));// / stdev);
    }
}

// scale a signal by a constant value
void scale_signal(double* data, int n, double scale)
{
    int j;
    for(j=0; j<n; j++)
    {
        data[j] *= scale;// / stdev);
    }
}

// reset values of running signal sums
void initialize_running_sums()
{
    int j;
    for(j = 0; j < NR_ACTIVE_CHANNELS; j++)
    {
        running_sum[j] = 0.0;
        running_sqr_sum[j] = 0.0;
    }
    running_count = 0.0;
}

// get standard deviation given square sum, avg, and count of a signal
double getStdev(double sqr_sum, double count, double avg)
{
    return sqrt((sqr_sum / count) - avg * avg);
}

// update running rum using data from current packet.
void update_running_sums(double* avgs, double* stdevs)
{
    int c,j;

    for(j = 0; j < NMAX; j++)
    {
        for(c = 0; c < NR_ACTIVE_CHANNELS; c++)
        {
            running_sum[c]      += channels[c][j];
            running_sqr_sum[c]  += (channels[c][j] * channels[c][j]);
        }
        running_count += 1.0;
    }

    for(c = 0; c < NR_ACTIVE_CHANNELS; c++)
    {
        avgs[c]     = (running_sum[c] / running_count);
        stdevs[c]   = getStdev(running_sqr_sum[c], running_count, avgs[c]);
    }

    if(running_count > 100000) initialize_running_sums();
}

// initializing running sums for bin energies
initialize_running_energies(int nbins)
{
    int k, c;

    for(k = 0; k < nbins; k++)
    {
        running_energy_sum[k]       = 0.0;
        running_energy_sqr_sum[k]  = 0.0;
        running_energy_count    = 0.0;
    }
}

// subtract channels from each other
// each channel is replaced by the difference between
// it and the average of all other channels
void calculate_differences_in_channels()
{
    double sum[NMAX];
    int j, c;
    double nch = (double)NR_ACTIVE_CHANNELS;

    for(j = 0; j < NMAX; j++)
    {
        sum[j] = 0.0;
        for(c = 0; c < NR_ACTIVE_CHANNELS; c++)
        {
            sum[j] += channels[c][j];
        }
        for(c = 0; c < NR_ACTIVE_CHANNELS; c++)
        {
            // we want c[j] = c[j] - ((sum(c[:]) - c[j]) / NR_ACTIVE_CHANNELS)
            // = ((NR_ACTIVE_CHANNELS + 1) c[j] - sum(c[:])) / NR_ACTIVE_CHANNELS

            channels[c][j] =
                (channels[c][j] * (nch + 1.0) - sum[j] * 1.0f) / nch;
            // printf("%f = (%f * %f - %f ) / %f\n",
            //     channels[c][j], channels[c][j], (nch + 1.0), sum[j], nch);
        }
    }
}

// update running energy sum/count using data
// from current packet.
void update_running_energies(double *bin_energies, int nbins, double* running_avg_energy)
{
    int k, c;

    for(k = 0; k < nbins; k++)
    {
        for(c = 0; c < NR_ACTIVE_CHANNELS; c++)
        {
            double energy           = bin_energies[c * NR_ACTIVE_CHANNELS + k];
            running_energy_sum[k]       += energy;
            running_energy_sqr_sum[k]  += (energy * energy);
            running_energy_count    += 1.0;
        }
    }
    for(k = 0; k < nbins; k++)
    {
        running_avg_energy[k] = running_energy_sum[k] / running_energy_count;
    }
}

// saturate a double
double saturate(double f)
{
    return (f > 1.0 ? 1.0 : (f < 0.0 ? 0.0 : f));
}

// swap two signals
void swap_data(double* data0, double* data1, int n)
{
    int j;
    for(j=0; j<n; j++)
    {
        double d = data0[j];
        data0[j] = data1[j];
        data1[j] = d;
    }
}

// main function, analyzes a packet of 4 channels
void analyze_msg(msg* m, double* freqs, bin* bins, int nbins)
{
    int chId, k;
    double avgs[4], stdevs[4];
    double *bin_energies, *max_energies;
    double *running_avg_energy;

    bin_energies        = (double*) malloc(nbins * NR_ACTIVE_CHANNELS * sizeof(double));
    max_energies        = (double*) malloc(nbins *                  1 * sizeof(double));
    running_avg_energy  = (double*) malloc(nbins *                  1 * sizeof(double));

    // -- fetch channel data -- //
    for (chId = 0; chId < NR_ACTIVE_CHANNELS; ++chId)
    {
        //printf("-- %d --\n", chId);
        //print_raw_channel(m->data[chId], NMAX);
        to_double(m->data[chId], channels[chId], NMAX);
    }

    update_running_sums(avgs, stdevs);

    // -- analyze channel data -- //
    for (chId = 0; chId < NR_ACTIVE_CHANNELS; ++chId)
    {
        remove_dc_component(channels[chId], NMAX, avgs[chId], stdevs[chId]);
        scale_signal(channels[chId], NMAX, 1500.0/avgs[chId]);
    }

    calculate_differences_in_channels();

    // for (chId = 0; chId < NR_ACTIVE_CHANNELS; ++chId)
    // {
    //     printf("-- %d --\n", chId);
    //     print_array(channels[chId], 16);
    // }

    for (chId = 0; chId < NR_ACTIVE_CHANNELS; ++chId)
    {
        //printf("% 5.1f %4.1f | ", avgs[chId], stdevs[chId]);
        {   // -- compute FFT -- //
            to_complex(channels[chId], NMAX);
            cdft(NMAX * 2, -1, channels[chId]); // -1 for some reason matches python/numpy
            complex_magnitude(channels[chId], NMAX);
        }
        analyze_bin(channels[chId], freqs, NMAX, bins, nbins, &(bin_energies[chId * NR_ACTIVE_CHANNELS]));
    }

    update_running_energies(bin_energies, nbins, running_avg_energy);
    // -- display information -- //

    for(k = 0; k < nbins; k++)
    {
        int best_channel = 0;
        max_energies[k] = 0.0;
        for (chId = 0; chId < NR_ACTIVE_CHANNELS; ++chId)
        {
            double cur_bin_energy = bin_energies[chId * NR_ACTIVE_CHANNELS + k];

            if(cur_bin_energy > max_energies[k])
            {
                max_energies[k] = cur_bin_energy;
                best_channel = chId;

                //printf("%0.2f\n", cur_bin_energy);
            }

            //printf(" %s", bin_energies[chId * NR_ACTIVE_CHANNELS + k] >= 20.0 ? "#": " ");
            //printf(" %3.0f", bin_energies[chId * NR_ACTIVE_CHANNELS + k]);
        }

        float saturated_energy = saturate(max_energies[k] / 7500.0);

        if(best_channel == 2)       best_channel = 3;
        else if(best_channel == 3)  best_channel = 2;

        visualize_send(
            (uint16_t)k, 
            (uint16_t)best_channel, 
            (uint16_t)(65535.0 * saturated_energy));

        //printf("%f\n", max_energies[k]);

        // if(max_energies[k] >= (10.0 * running_avg_energy[k]))
        // {
        //     visualize_send( k, best_channel, );
        //     printf("%d ", best_channel);
        // }else
        // {
        //     printf("  ");
        // }
    }
    //printf("\n");

    free(bin_energies);
    free(max_energies);
    free(running_avg_energy);
}