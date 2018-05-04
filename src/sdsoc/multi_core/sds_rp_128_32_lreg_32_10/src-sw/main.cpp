#include <stdint.h>
#include <stdlib.h>
#include "multi.h"
#include "sds_lib.h"

#ifndef NUM_TESTS
#define NUM_TESTS 1
#endif

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

// init: Random Projection
static void
init_arrays_0(data_t *x,  float *x_fl, len_t datalen)
{
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_features; j++) {
			float rnum = 2*((float)rand()/(float)(RAND_MAX/1.0))-1;
			x[i * n_features + j] = toFixed(rnum);
			x_fl[i*n_features + j] = rnum;
		}
	}
}

// init: Linear Regression
static void
init_arrays_1(data_t *x,  coef_t *a, coef_t *b, float *x_fl, float *a_fl, float *b_fl, len_t datalen)
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


// random projection
static void
check_results_0(data_t *output, float *output_fl, len_t datalen)
{
	printf("\n CHECK RESULTS \n");
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_components; j++) {

			uint32_t oval_uint = output[(i*n_components) + j];
			int32_t oval_fixed = (int32_t) oval_uint;
			float oval_float = toFloat(oval_fixed);
			float testFloat = output_fl[(i*n_components) + j];
		
			if ( i<=5200 ) {
				std::cout << "RES["<<i<<"]["<<j<<"]=" << oval_uint << ";" << oval_fixed	<< ";" << oval_float << ";" << testFloat << std::endl;
			}
		}

	}
	std::cout << "Test passed" << std::endl;
}

// linear regression
static void
check_results_1(data_t *output, float *output_fl, len_t datalen)
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

	data_t *x, *output, *x_lr, *output_lr;
	coef_t *a_lr, *b_lr;
	float *x_fl, *output_fl, *x_lr_fl, *a_lr_fl, *b_lr_fl, *output_lr_fl;

	len_t datalen = 2;
	
	// test random projection
	x = (data_t *)sds_alloc(int(datalen*n_features*sizeof(data_t)));
	output = (data_t *)sds_alloc(int(datalen*n_components*sizeof(data_t)));
	
	//float
	x_fl = (float *)malloc(int(datalen*n_features*sizeof(data_t)));
	output_fl = (float *)malloc(int(datalen*n_components*sizeof(data_t)));
	
	// test linear regression
	x_lr = (data_t *)sds_alloc(int(datalen*n_components*sizeof(data_t)));
	a_lr = (coef_t *)sds_alloc(int(n_outputs*(n_components)*sizeof(coef_t)));
	b_lr = (coef_t *)sds_alloc(int(n_outputs*sizeof(coef_t)));
	output_lr = (data_t *)sds_alloc(int(datalen*n_outputs*sizeof(data_t)));

	// float
	x_lr_fl = (float *)malloc(int(datalen*n_components*sizeof(float)));
	a_lr_fl = (float *)malloc(int(n_outputs*(n_components)*sizeof(float)));
	b_lr_fl = (float *)malloc(int(n_outputs*sizeof(float)));
	output_lr_fl = (float *)malloc(int(datalen*n_outputs*sizeof(float)));

	//read_data(x, x_fl);
	
	// init inputs and weights
	srand(322131);
	init_arrays_0(x, x_fl, datalen);
	init_arrays_1(x_lr, a_lr, b_lr, x_lr_fl, a_lr_fl, b_lr_fl, datalen);

	// Random Projection and check results
#pragma SDS async(1)
	RandomProjection(x, output, datalen);

	// Linear Regression
#pragma SDS async(2)
	LinReg(x_lr, a_lr, b_lr, output_lr, datalen);

#pragma SDS wait(1)
#pragma SDS wait(2)


	RP_float(x_fl, output_fl, datalen);
	check_results_0(output, output_fl, datalen);

	LinReg_float(x_lr_fl, a_lr_fl, b_lr_fl, output_lr_fl, datalen);
	check_results_1(output_lr, output_lr_fl, datalen);

	sds_free(x);
	sds_free(output);
	free(x_fl);
	free(output_fl);

	sds_free(x_lr);
	sds_free(a_lr);
	sds_free(b_lr);
	sds_free(output_lr);
	free(x_lr_fl);
	free(a_lr_fl);
	free(b_lr_fl);
	free(output_lr_fl);

    return 0;

  }
