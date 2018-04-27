#include <stdint.h>
#include <stdlib.h>
#include "rp.h"
#include "sds_lib.h"

#ifndef NUM_TESTS
#define NUM_TESTS 1
#endif

static void
RP_float(float *x, float *output, len_t datalen)
{
	// seeds
	const unsigned int seeds[n_outputs] = {
		#include "seeds32.dat"
	};

	static float accum[n_outputs];
	static ap_uint<32> lfsr[n_outputs];

	int rp_mat[n_features][n_outputs];

	for (int i=0; i<datalen; i++) {

		printf("\n DEBUG: \n");
		for (int k=0; k<n_outputs; k++) {
			accum[k] = 0;
			lfsr[k] = seeds[k];
			printf("%zu ", seeds[k]);
		}
		printf("\n DEBUG \n");

		for (int j=0; j<n_features; j++) {

			float xin;
			xin = x[i*n_features + j];

			for (int k=0; k<n_outputs; k++) {

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
		for (int k=0; k<n_outputs; k++) {
			output[i*n_outputs + k] = accum[k]*0.1767766953;
		}


		// check rp_mat
		for (int j=0; j<n_features; j++) {
			for (int k=0; k<n_outputs; k++) {
				printf("%d,", rp_mat[j][k]*2 -1);
			}
			printf("\n");
		}
	}
}


static void 
read_data(data_t *x, float *x_fl)
{
	
	// must match with dataset
	int N = 2;
	int nf = 128;
	int no = 32;
	
	float xi[1];
	float ai[1];
	float bi[1];
	
	float ival_float;
	int32_t ival_fix;
	uint32_t ival_uint;
	
	FILE *xin, *ain, *bin;
    xin = fopen( "x_rand.dat","r" );
	
	printf("\n input x \n");
	
	// x_fl
	for (int i=0; i<N*nf; i++) {
		fscanf(xin, "%f", &xi[0]);
		
		/* data-types */
		ival_float = xi[0];
		ival_fix = toFixed(ival_float);
		ival_uint = (uint32_t) ival_fix;
		printf("%f;%d;%zu\n", ival_float, ival_fix, ival_uint);
		
		x_fl[i] = ival_float;
		x[i] = ival_uint;
	}
	
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

static void
init_arrays(data_t *x,  float *x_fl, len_t datalen)
{
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_features; j++) {
			//float rnum = 2*((float)rand()/(float)(RAND_MAX/1.0))-1;
			x[i * n_features + j] = rand();
		}
	}
}

static void
check_results(data_t *output, data_t *output_sw, float *output_fl, data_t *x, len_t datalen)
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

int main()
{

	data_t *x, *output, *output_sw;
	float *x_fl, *output_fl;

	len_t bsize = 1000;
	len_t datalen = 2;
	
	// fixed-point 
	x = (data_t *)sds_alloc(int(bsize*n_features*sizeof(data_t)));
	output = (data_t *)sds_alloc(int(bsize*n_outputs*sizeof(data_t)));
	output_sw = (data_t *)malloc(int(bsize*n_outputs*sizeof(data_t)));
	
	//float
	x_fl = (float *)malloc(int(bsize*n_features*sizeof(data_t)));
	output_fl = (float *)malloc(int(bsize*n_outputs*sizeof(data_t)));
	
	//read_data(x, x_fl);
	
	// init inputs and weights
	srand(322131);
	init_arrays(x, x_fl, datalen);

	RandomProjection(x, output, datalen);
	RP_float(x_fl, output_fl, datalen);

	check_results(output, output_sw, output_fl, x, datalen);

	sds_free(x);
	free(x_fl);
	sds_free(output);
	free(output_sw);
	free(output_fl);

    return 0;

  }
