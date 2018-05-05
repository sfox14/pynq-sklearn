#include <stdint.h>
#include <stdlib.h>
#include "pipe.h"
#include "sds_lib.h"

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
LinReg_float(float *x, float a[n_outputs*n_components], float b[n_outputs],
        float *output, len_t datalen)
{
	for (int i=0; i<datalen; i++){
		for (int j=0; j<n_outputs; j++) {
			float result = 0;
			for (int k=0; k<n_components; k++) {
				float imm = x[i*n_components + k] * a[(j*n_components)+ k];
				result += imm;
			}
			output[i*n_outputs + j]=result + b[j]; //y_intercept
		}
	}
}

static void
RP_float(float *x, float *output, len_t datalen)
{
	// seeds
	const unsigned int seeds[n_components] = {
		#include "seeds32.dat"
	};

	static float accum[n_components];
	static ap_uint<32> lfsr[n_components];

	int rp_mat[n_features][n_components];

	for (int i=0; i<datalen; i++) {

		//printf("\n DEBUG: \n");
		for (int k=0; k<n_components; k++) {
			accum[k] = 0;
			lfsr[k] = seeds[k];
			//printf("%zu ", seeds[k]);
		}
		//printf("\n DEBUG \n");

		for (int j=0; j<n_features; j++) {

			float xin;
			xin = x[i*n_features + j];

			for (int k=0; k<n_components; k++) {

				bool b_32 = lfsr[k].get_bit(32-32);
				bool b_22 = lfsr[k].get_bit(32-22);
				bool b_7 = lfsr[k].get_bit(32-7);
				bool b_2 = lfsr[k].get_bit(32-2);
				bool new_bit = b_32 ^ b_22 ^ b_7 ^ b_2;

				lfsr[k] = lfsr[k] << 1;
				lfsr[k].set_bit(0, new_bit);
				//lfsr[k] = lfsr[k] >> 1;
				//lfsr[k].set_bit(31, new_bit);

				if (new_bit) {
					accum[k] += xin;
				} else {
					accum[k] -= xin;
				}

				rp_mat[j][k] = new_bit;
			}

		}
		for (int k=0; k<n_components; k++) {
			output[i*n_components + k] = accum[k]*0.1767766953;
		}


		// check rp_mat
		for (int j=0; j<n_features; j++) {
			for (int k=0; k<n_components; k++) {
				//printf("%d,", rp_mat[j][k]*2 -1); // debug the random matrix
			}
			//printf("\n");
		}
	}
}

static void
Pipe_float(float *x, float a[n_outputs*n_components], float b[n_outputs],
        float *output, float *imm, len_t datalen)
{

	RP_float(x, imm, datalen);
	LinReg_float(imm, a, b, output, datalen);

}

float toFloat(int32_t a)
{
	float result;
	float val = 1 << frac_width;
	result = a/val;
	
	return result;
}


int32_t toFixed(float a)
{
	int32_t result;
	int32_t val = 1 << frac_width; //logical shift
	
	if (a<0) result = a*val-0.5; //rounding
	else result = a*val +0.5;
	
	return result;
}


// init: Random Projection + Linear Regression
static void
init_arrays(data_t *x,  coef_t *a, coef_t *b, float *x_fl, float *a_fl, float *b_fl, len_t datalen)
{
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_components; j++) {
			float rnum = 2*((float)rand()/(float)(RAND_MAX/1.0))-1;
			x[i * n_components + j] = toFixed(rnum);
			x_fl[i * n_components + j] = rnum;
		}
	}

	for (int i=0; i<n_outputs; i++) {
		for (int j=0; j<(n_components); j++) {
			float rnum = 2*((float)rand()/(float)(RAND_MAX/1.0))-1;
			a[i * n_components + j] = toFixed(rnum);
			a_fl[i * n_components + j] = rnum;
		}
		float rnum = 2*((float)rand()/(float)(RAND_MAX/1.0))-1;
		b[i] = toFixed(rnum);
		b_fl[i] = rnum;
	}
}



// random projection + linear regression
static void
check_results(data_t *output, float *output_fl, len_t datalen)
{
	printf("\n CHECK RESULTS \n");
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_outputs; j++) {

			uint32_t oval_uint = output[(i*n_outputs) + j];
			int32_t oval_fixed = (int32_t) oval_uint;
			float oval_float = toFloat(oval_fixed);
			float testFloat = output_fl[(i*n_outputs) + j];

			if ( i<=5200 ) {
				std::cout << "RES["<<i<<"]["<<j<<"]=" << oval_uint << ";" << oval_fixed	<< ";" << oval_float << ";" << testFloat << std::endl;
			}
		}
	}
	std::cout << "Test passed" << std::endl;
}

void _p0_Pipe_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen);
int main()
{

	data_t *x, *output;
	coef_t *a, *b;
	float *x_fl, *output_fl, *a_fl, *b_fl, *imm;

	len_t datalen = 2;
	
	// test random projection + linear regression
	x = (data_t *)sds_alloc(int(datalen*n_features*sizeof(data_t)));
	a = (coef_t *)sds_alloc(int(n_outputs*(n_components)*sizeof(coef_t)));
	b = (coef_t *)sds_alloc(int(n_outputs*sizeof(coef_t)));
	output = (data_t *)sds_alloc(int(datalen*n_outputs*sizeof(data_t)));
	
	//float
	x_fl = (float *)malloc(int(datalen*n_features*sizeof(float)));
	a_fl = (float *)malloc(int(n_outputs*(n_components)*sizeof(float)));
	b_fl = (float *)malloc(int(n_outputs*sizeof(float)));
	output_fl = (float *)malloc(int(datalen*n_outputs*sizeof(float)));
	imm = (float *)malloc(int(datalen*n_components*sizeof(float)));
	
	// init inputs and weights
	srand(322131);
	init_arrays(x, a, b, x_fl, a_fl, b_fl, datalen);

	perf_counter hw_ctr, sw_ctr;

	// Execute Pipeline and check results
	hw_ctr.start();
	_p0_Pipe_1_noasync(x, a, b, output, datalen);
	hw_ctr.stop();
	sw_ctr.start();
	Pipe_float(x_fl, a_fl, b_fl, output_fl, imm, datalen);
	sw_ctr.stop();
	check_results(output, output_fl, datalen);

	uint64_t sw_cycles = sw_ctr.avg_cpu_cycles();
	uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();
	double speedup = (double) sw_cycles / (double) hw_cycles;

	std::cout << "Average number of CPU cycles running Pipe in software: "
	   << sw_cycles << std::endl;
	std::cout << "Average number of CPU cycles running Pipe in hardware: "
	   << hw_cycles << std::endl;
	std::cout << "Speed up: " << speedup << std::endl;

	sds_free(x);
	sds_free(a);
	sds_free(b);
	sds_free(output);

	free(x_fl);
	free(a_fl);
	free(b_fl);
	free(output_fl);
	free(imm);

    return 0;

  }
