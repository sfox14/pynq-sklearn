#include <stdint.h>
#include <stdlib.h>
#include "linear.h"
#include <sds_lib.h>

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
	op_t op1, op2, bop, imm;

	for (int i=0; i<datalen; i++){
		for (int j=0; j<n_outputs; j++) {
			op_t result = 0;
			for (int k=0; k<n_features; k++) {
				data_t xval_uint = x[i*n_features + k];
				coef_t aval_uint = a[(j*n_features)+ k];
				op1.V = xval_uint;
				op2.V = aval_uint;
				result += (op1*op2);
			}
			coef_t bval_uint = b[j];
			bop.V = bval_uint(bit_width-1,0);
			imm = result + bop;
			output[i*n_outputs + j] = imm.V;
		}
	}
}

static void
LinReg_float(float *x, float a[n_outputs*n_features], float b[n_outputs],
        float *output, len_t datalen)
{
	for (int i=0; i<datalen; i++){
		for (int j=0; j<n_outputs; j++) {
			float result = 0;
			//fp_t result = 0;
			for (int k=0; k<n_features; k++) {
				float imm = x[i*n_features + k] * a[(j*n_features)+ k];
				result += imm;
			}
			output[i*n_outputs + j]=result + b[j]; //y_intercept
		}
	}
}


static void
read_data(data_t *x, coef_t *a, coef_t *b, float *x_fl, float *a_fl, float *b_fl)
{

	// must match with dataset
	int N = 10;
	int nf = 32;
	int no = 10;

	float xi[1];
	float ai[1];
	float bi[1];

	float ival_float;
	int32_t ival_fix;
	uint32_t ival_uint;

	FILE *xin, *ain, *bin;
    xin = fopen( "x_rand.dat","r" );
    ain = fopen( "a_rand.dat","r" );
	bin = fopen( "b_rand.dat", "r" );

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

	printf("\n parameter a \n");

	// a_fl
	for (int i=0; i<no*nf; i++) {
		fscanf(ain, "%f", &ai[0]);

		/* data-types */
		ival_float = ai[0];
		ival_fix = toFixed(ival_float);
		ival_uint = (uint32_t) ival_fix;
		printf("%f;%d;%zu\n", ival_float, ival_fix, ival_uint);

		a_fl[i] = ival_float;
		a[i] = ival_uint;
	}

	printf("\n parameter b \n");

	// b_fl
	for (int i=0; i<no; i++) {
		fscanf(bin, "%f", &bi[0]);

		/* data-types */
		ival_float = bi[0];
		ival_fix = toFixed(ival_float);
		ival_uint = (uint32_t) ival_fix;
		printf("%f;%d;%zu\n", ival_float, ival_fix, ival_uint);

		b_fl[i] = ival_float;
		b[i] = ival_uint;
	}
	printf("\n");
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
init_arrays(data_t *x,  coef_t *a, coef_t *b, float *x_fl, float *a_fl, float *b_fl, len_t datalen)
{
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_features; j++) {
			float rnum = 2*((float)rand()/(float)(RAND_MAX/1.0))-1;
			x[i * n_features + j] = rnum; //% (datalen * n_features);
		}
	}

	for (int i = 0; i < n_outputs; i++) {
		for (int j = 0; j < n_features; j++) {
			float rnum = 2*((float)rand()/(float)(RAND_MAX/1.0))-1;
			a[i * n_features + j] = rnum;
		}
		b[i]=0;
	}
}

static void
check_results(data_t *output, data_t *output_sw, float *output_fl, len_t datalen)
{
	for (int i = 0; i < datalen; i++) {
		for (int j = 0; j < n_outputs; j++) {

			uint32_t oval_uint = output[(i*n_outputs) + j];
			int32_t oval_fixed = (int32_t) oval_uint;
			float oval_float = toFloat(oval_fixed);

			if ( i<=5200 ) {
				std::cout << output[(i*n_outputs) + j] << ";" << output_sw[(i*n_outputs) + j]
								<< ";" << output_fl[(i*n_outputs) + j] << ";" << oval_uint
								<< ";" << oval_fixed << ";" << oval_float << std::endl;
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
	float *a_fl, *b_fl, *x_fl, *output_fl;

	len_t datalen = 10;
	
	a = (coef_t *)sds_alloc(int(n_outputs*n_features*sizeof(coef_t)));
	b = (coef_t *)sds_alloc(int(n_outputs*sizeof(coef_t)));
	x = (data_t *)sds_alloc(int(bsize*n_features*sizeof(data_t)));
	output = (data_t *)sds_alloc(int(bsize*n_outputs*sizeof(data_t)));
	output_sw = (data_t *)malloc(int(bsize*n_outputs*sizeof(data_t)));
	
	//float
	a_fl = (float *)malloc(int(n_outputs*n_features*sizeof(coef_t)));
	b_fl = (float *)malloc(int(n_outputs*sizeof(coef_t)));
	x_fl = (float *)malloc(int(bsize*n_features*sizeof(data_t)));
	output_fl = (float *)malloc(int(bsize*n_outputs*sizeof(data_t)));

	read_data(x, a, b, x_fl, a_fl, b_fl);

	// init inputs and weights
	//srand(322131);
	//init_arrays(x, a, b, x_fl, a_fl, b_fl, datalen);

    std::cout << x[0] << ";" << x[1] << ";" << x[10] << ";" << a[10] << std::endl;

    perf_counter hw_ctr, sw_ctr;

	hw_ctr.start();
	LinReg(x, a, b, output, datalen);
	hw_ctr.stop();
	sw_ctr.start();
	LinReg_sw(x, a, b, output_sw, datalen);
	sw_ctr.stop();
	LinReg_float(x_fl, a_fl, b_fl, output_fl, datalen);

	check_results(output, output_sw, output_fl, datalen);

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
	free(x_fl);
	free(a_fl);
	free(b_fl);
	free(output_fl);

    return 0;

  }
