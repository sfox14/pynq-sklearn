#include <stdint.h>
#include <stdlib.h>
#include <sds_lib.h>
#include "linear.h"
#include <time.h>

#ifndef NUM_TESTS
#define NUM_TESTS 1
#endif

#define SEC_TO_NS (1000000000)

class perf_counter
{
public:
     uint64_t tot, cnt, calls;
     perf_counter() : tot(0), cnt(0), calls(0) {};
     inline void reset() { tot = cnt = calls = 0; }
     inline void start() { cnt = sds_clock_counter(); calls++; };
     inline void stop() { tot += (sds_clock_counter() - cnt); };
     inline uint64_t avg_cpu_cycles() { return ((tot+(calls>>1)) / calls); };
};

static void
init_arrays(data_t *x,  coef_t *a, coef_t *b, len_t datalen)
{
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_features; j++) {
			x[i * n_features + j] = rand() % (datalen * n_features);
		}
	}

	for (int i = 0; i < n_outputs; i++) {
		for (int j = 0; j < n_features; j++) {
			a[i * n_features + j] = i+j;
		}
		b[i]=0;
	}
}


void _p0_LinReg_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen);

int main()
{

	coef_t *a;
	coef_t *b;
	data_t *x, *output, *output_sw;

	len_t datalen = 50;
	
	a = (coef_t *)sds_alloc(int(n_outputs*n_features*sizeof(coef_t)));
	b = (coef_t *)sds_alloc(int(n_outputs*sizeof(coef_t)));
	x = (data_t *)sds_alloc(int(bsize*n_features*sizeof(data_t)));
	output = (data_t *)sds_alloc(int(bsize*n_outputs*sizeof(data_t)));
	output_sw = (data_t *)malloc(int(bsize*n_outputs*sizeof(data_t)));
	

	// init inputs and weights
	init_arrays(x, a, b, datalen);

    struct timespec * start = (struct timespec *)malloc(sizeof(struct timespec));
    struct timespec * stop = (struct timespec *)malloc(sizeof(struct timespec));

    //perf_counter hw_ctr, sw_ctr;
    //hw_ctr.start();
    // Start the timer
    clock_gettime(CLOCK_REALTIME, start);
    for (int i=0; i<2000; i++) {
        _p0_LinReg_1_noasync(x, a, b, output, datalen);
    }
    // Stop the timer
    clock_gettime(CLOCK_REALTIME, stop);

    //hw_ctr.stop();

    //uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();
    //std::cout << "Average number of CPU cycles running Linear in hardware: "
	//   << hw_cycles << std::endl;

    int totalTime = (stop->tv_sec*SEC_TO_NS + stop->tv_nsec) - (start->tv_sec*SEC_TO_NS + start->tv_nsec);
    printf("time = %f s\n", ((float)totalTime/1000000000));

    return 0;

  }
