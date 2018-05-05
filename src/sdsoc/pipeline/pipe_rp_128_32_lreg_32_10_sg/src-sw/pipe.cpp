
#include "pipe.h"

// read
void read(data_t *x, fifoB_t &xbuf, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<n_features; j++) {
#pragma HLS PIPELINE
			xbuf << x[i*n_features + j];
		}
	}
}


// write
void write(fifoB_t &obuf, data_t *output, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<n_outputs; j++) {
#pragma HLS PIPELINE
			obuf >> output[i*n_outputs + j];
		}
	}
}


// Top-Level function
void Pipe(data_t *x, coef_t a[n_outputs*n_components], coef_t b[n_outputs],
		data_t *output, len_t datalen)
{

	len_t iter;
	fifoA_t rbuf;
	fifoB_t obuf;
	fifoB_t xbuf;

	coef_t amem[n_outputs][n_components], bmem[n_outputs];

	// load weights and biases
	iter = 2; //datalen;
	for (int i=0; i<n_outputs; i++) {
		for (int j=0; j<n_components; j++) {
#pragma HLS PIPELINE
			amem[i][j] = a[i*n_components + j];
		}
	}

	for (int i=0; i<n_outputs; i++) {
#pragma HLS PIPELINE
		bmem[i] = b[i];
	}

#pragma HLS dataflow

	read(x, xbuf, iter);
	BinaryRP(xbuf, rbuf, iter);
	LinearRegression(rbuf, amem, bmem, obuf, iter);
	write(obuf, output, iter);

}




