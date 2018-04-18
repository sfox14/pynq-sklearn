#include <stdint.h>
#include <stdlib.h>
#include <sds_lib.h>
#include "linear.h"

#ifndef NUM_TESTS
#define NUM_TESTS 1
#endif

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
LinReg_sw(data_t *x, coef_t a[n_outputs*n_features], coef_t b[n_outputs],
        data_t *output, len_t datalen)
{
	for (int i=0; i<datalen; i++){
		for (int j=0; j<n_outputs; j++) {
			data_t result = 0;
			for (int k=0; k<n_features; k++) {
				data_t imm = x[i*n_features + k] * a[(j*n_features) + k];
				result += imm;
			}
			output[i*n_outputs + j]=result + b[j]; //y_intercept
		}
	}
}

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

static void
check_results(data_t *output, data_t *output_sw, len_t datalen)
{
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_outputs; j++) {

			if ( i<=5200 ) {
				std::cout << output[(i*n_outputs) + j] << ";" << output_sw[(i*n_outputs) + j] << std::endl;
			}
			if (output[(i*n_outputs) + j] != output_sw[(i*n_outputs) + j]) {
				std::cout   << "Mismatch: LinReg(" << i << ") = "
							<< output[(i*n_outputs) + j] << ";  LinReg_sw(" << i << ") = "
							<< output_sw[(i*n_outputs) + j] << std::endl;
				std::cout << "Test failed" << std::endl;
				if (i>5200) {
					return;
				}
			}
		}

	}
	std::cout << "Test passed" << std::endl;
}

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

    std::cout << x[0] << ";" << x[1] << ";" << x[10] << ";" << a[10] << std::endl;

    perf_counter hw_ctr, sw_ctr;

	hw_ctr.start();
	LinReg(x, a, b, output, datalen);
	hw_ctr.stop();
	sw_ctr.start();
	LinReg_sw(x, a, b, output_sw, datalen);
	sw_ctr.stop();

	check_results(output, output_sw, datalen);
	std::cout << output[484] << ";" << output_sw[484] << std::endl;

	uint64_t sw_cycles = sw_ctr.avg_cpu_cycles();
	uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();
	double speedup = (double) sw_cycles / (double) hw_cycles;

	std::cout << "Average number of CPU cycles running Linear in software: "
	   << sw_cycles << std::endl;
	std::cout << "Average number of CPU cycles running Linear in hardware: "
	   << hw_cycles << std::endl;
	std::cout << "Speed up: " << speedup << std::endl;

	sds_free(x);
	sds_free(a);
	sds_free(b);
	sds_free(output);
	free(output_sw);

    return 0;

  }
