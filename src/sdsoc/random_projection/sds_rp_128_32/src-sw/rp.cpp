
#include "rp.h"

void read(data_t *x, fifoA_t &xbuf, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<(n_features); j++) {
#pragma HLS PIPELINE
			xbuf << x[i*n_features + j];
		}
	}
}

void write(fifoB_t &obuf, data_t *output, len_t datalen)
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

	read(x, xbuf, iter);
	BinaryRP(xbuf, obuf, iter);
	write(obuf, output, iter);

  }





