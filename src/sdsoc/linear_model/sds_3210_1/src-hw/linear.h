
#include <assert.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <hls_stream.h>
#include <ap_int.h>
using namespace std;

#define bsize 1000

/* Specify architecture parameters */
#define n_features		32      //160
#define n_outputs		10
#define in_width 		1		// fifoA egress stream width
#define out_width		1		// fifoB ingress stream width

#define kn				10		// number of dot-products
#define kf				10		// Unroll factor of N
#define kp				1		// Pipeline factor of N
#define bit_width		32 		// bit width

const int instream_width = in_width*bit_width;
const int outstream_width = out_width*bit_width;

/* Specify data types */
typedef ap_uint<bit_width>	coef_t;
typedef ap_uint<bit_width>	data_t;
typedef int			len_t;
typedef ap_uint<instream_width> parA_t;
typedef ap_uint<outstream_width> parB_t;
typedef hls::stream<parA_t> fifoA_t;	// read fifo
typedef hls::stream<parB_t> fifoB_t;	// write fifo


// Function definition
#pragma SDS data access_pattern(x:SEQUENTIAL, output:SEQUENTIAL)
#pragma SDS data copy(x[0:(datalen*n_features)], output[0:(datalen*n_outputs)])
#pragma SDS data data_mover(x:AXIDMA_SIMPLE, a: AXIDMA_SIMPLE, b: AXIDMA_SIMPLE, output:AXIDMA_SIMPLE)
#pragma SDS data mem_attribute(x:PHYSICAL_CONTIGUOUS, a:PHYSICAL_CONTIGUOUS, b:PHYSICAL_CONTIGUOUS, output:PHYSICAL_CONTIGUOUS)
void LinReg(data_t *x, coef_t a[n_outputs*n_features], coef_t b[n_outputs],
		data_t *output, len_t datalen);
