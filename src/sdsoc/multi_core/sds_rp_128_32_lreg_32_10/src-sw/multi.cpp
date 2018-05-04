
#include "multi.h"

// Read: Random Projection
void read_0(data_t *x, fifoA_t &xbuf, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<n_features; j++) {
#pragma HLS PIPELINE
			xbuf << x[i*n_features + j];
		}
	}
}

// Read: Linear Regression
void read_1(data_t *x, fifoA_t &xbuf, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<(n_components); j++) {
#pragma HLS PIPELINE
			xbuf << x[i*n_components + j];
		}
	}
}

// Write: Random Projection
void write_0(fifoB_t &obuf, data_t *output, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<n_components; j++) {
#pragma HLS PIPELINE
			obuf >> output[i*n_components + j];
		}
	}
}

// Write: Linear Regression
void write_1(fifoB_t &obuf, data_t *output, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<n_outputs; j++) {
#pragma HLS PIPELINE
			obuf >> output[i*n_outputs + j];
		}
	}
}


// Top-level function with class instantiated
void RandomProjection(data_t *x, data_t *output, len_t datalen)
  {

	len_t iter;
	fifoA_t xbuf;
	fifoB_t obuf;

	iter = datalen;

#pragma HLS dataflow

	read_0(x, xbuf, iter);
	BinaryRP(xbuf, obuf, iter);
	write_0(obuf, output, iter);

  }


// Top-level function with class instantiated
void LinReg(data_t *x, coef_t a[n_outputs*n_components], coef_t b[n_outputs],
		data_t *output, len_t datalen)
  {

	len_t iter;
	fifoA_t xbuf;
	fifoB_t obuf;

	coef_t amem[n_outputs][n_components], bmem[n_outputs];

	// load weights and biases
	iter = datalen;
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

	read_1(x, xbuf, iter);
	LinearRegression(xbuf, amem, bmem, obuf, iter);
	write_1(obuf, output, iter);

  }




