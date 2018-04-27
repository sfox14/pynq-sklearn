
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

#include "cf_stub.h"
extern "C" void _p0_RandomProjection_1_noasync(data_t * x, data_t * output, len_t datalen);
extern "C" void _p0_RandomProjection_1_noasync(data_t * x, data_t * output, len_t datalen)
{
  switch_to_next_partition(0);
  int start_seq[1];
  start_seq[0] = 0;
  cf_request_handle_t _p0_swinst_RandomProjection_1_cmd;
  cf_send_i(&(_p0_swinst_RandomProjection_1.cmd_RandomProjection), start_seq, 1 * sizeof(int), &_p0_swinst_RandomProjection_1_cmd);
  cf_wait(_p0_swinst_RandomProjection_1_cmd);

  cf_send_i(&(_p0_swinst_RandomProjection_1.x_V), x, ((datalen*128)) * 4, &_p0_request_0);
  cf_send_i(&(_p0_swinst_RandomProjection_1.datalen), &datalen, 4, &_p0_request_2);

  cf_receive_i(&(_p0_swinst_RandomProjection_1.output_V), output, ((datalen*32)) * 4, &_p0_RandomProjection_1_noasync_num_output_V, &_p0_request_1);

  cf_wait(_p0_request_0);
  cf_wait(_p0_request_1);
  cf_wait(_p0_request_2);
}







