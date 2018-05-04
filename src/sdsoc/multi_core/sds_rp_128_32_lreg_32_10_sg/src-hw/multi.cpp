
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

  cf_send_i(&(_p0_swinst_RandomProjection_1.x_V), x, ((datalen*128)) * 4, &_p0_request_5);
  cf_send_i(&(_p0_swinst_RandomProjection_1.datalen), &datalen, 4, &_p0_request_7);

  cf_receive_i(&(_p0_swinst_RandomProjection_1.output_V), output, ((datalen*32)) * 4, &_p0_RandomProjection_1_noasync_num_output_V, &_p0_request_6);
  
  cf_wait(_p0_request_5);
  cf_wait(_p0_request_6);
  cf_wait(_p0_request_7);
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

#include "cf_stub.h"
extern "C" void _p0_LinReg_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen);
extern "C" void _p0_LinReg_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen)
{
  switch_to_next_partition(0);
  int start_seq[1];
  start_seq[0] = 0;
  cf_request_handle_t _p0_swinst_LinReg_1_cmd;
  cf_send_i(&(_p0_swinst_LinReg_1.cmd_LinReg), start_seq, 1 * sizeof(int), &_p0_swinst_LinReg_1_cmd);
  cf_wait(_p0_swinst_LinReg_1_cmd);

  cf_send_i(&(_p0_swinst_LinReg_1.x_V), x, ((datalen*32)) * 4, &_p0_request_0);
  cf_send_i(&(_p0_swinst_LinReg_1.a_V_PORTA), a, 1280, &_p0_request_1);
  cf_send_i(&(_p0_swinst_LinReg_1.b_V_PORTA), b, 40, &_p0_request_2);
  cf_send_i(&(_p0_swinst_LinReg_1.datalen), &datalen, 4, &_p0_request_4);

  cf_receive_i(&(_p0_swinst_LinReg_1.output_V), output, ((datalen*10)) * 4, &_p0_LinReg_1_noasync_num_output_V, &_p0_request_3);

  cf_wait(_p0_request_0);
  cf_wait(_p0_request_1);
  cf_wait(_p0_request_2);
  cf_wait(_p0_request_3);
  cf_wait(_p0_request_4);
}






