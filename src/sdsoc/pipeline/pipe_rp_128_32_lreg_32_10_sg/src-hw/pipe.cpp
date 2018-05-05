
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

#include "cf_stub.h"
extern "C" void _p0_Pipe_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen);
extern "C" void _p0_Pipe_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen)
{
  switch_to_next_partition(0);
  int start_seq[1];
  start_seq[0] = 0;
  cf_request_handle_t _p0_swinst_Pipe_1_cmd;
  cf_send_i(&(_p0_swinst_Pipe_1.cmd_Pipe), start_seq, 1 * sizeof(int), &_p0_swinst_Pipe_1_cmd);
  cf_wait(_p0_swinst_Pipe_1_cmd);

  cf_send_i(&(_p0_swinst_Pipe_1.x_V), x, ((datalen*128)) * 4, &_p0_request_0);
  cf_send_i(&(_p0_swinst_Pipe_1.a_V_PORTA), a, 1280, &_p0_request_1);
  cf_send_i(&(_p0_swinst_Pipe_1.b_V_PORTA), b, 40, &_p0_request_2);
  cf_send_i(&(_p0_swinst_Pipe_1.datalen), &datalen, 4, &_p0_request_4);

  cf_receive_i(&(_p0_swinst_Pipe_1.output_V), output, ((datalen*10)) * 4, &_p0_Pipe_1_noasync_num_output_V, &_p0_request_3);

  cf_wait(_p0_request_0);
  cf_wait(_p0_request_1);
  cf_wait(_p0_request_2);
  cf_wait(_p0_request_3);
  cf_wait(_p0_request_4);
}






