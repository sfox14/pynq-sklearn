
#include "linear.h"



void compute(fifoA_t &xbuf, coef_t amem[n_outputs][n_features], coef_t bmem[n_outputs],
		fifoB_t &obuf, len_t datalen)
{
	static data_t result[n_outputs];

#pragma HLS array_partition variable=amem complete dim=1
#pragma HLS array_partition variable=result complete
#pragma HLS array_partition variable=bmem complete


	for (int kk=0; kk<datalen; kk++) {

		//reset
reset:	for (int ir=0; ir<(n_outputs/10); ir++) {
#pragma HLS PIPELINE
			for (int jr=0; jr<10; jr++) {
#pragma HLS UNROLL
				result[ir*10 + jr] = 0;
			}
		}

		// macc
macc:	for (int i=0; i<n_features; i++) {
#pragma HLS PIPELINE
			parA_t xdat;
			xbuf >> xdat;
			for (int j=0; j<n_outputs; j++) {
				result[j] += xdat*amem[j][i];
			}
		}

		// result
res:	for (int ii=0; ii<n_outputs; ii++) {
#pragma HLS PIPELINE
			obuf << result[ii] + bmem[ii];
		}
	}

}

void read(data_t *x, fifoA_t &xbuf, len_t datalen)
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<n_features; j++) {
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
void LinReg(data_t *x, coef_t a[n_outputs*n_features], coef_t b[n_outputs],
		data_t *output, len_t datalen)
  {

	len_t iter;
	fifoA_t xbuf;
	fifoB_t obuf;

	coef_t amem[n_outputs][n_features], bmem[n_outputs];

	// load weights and biases
	iter = datalen;
	for (int i=0; i<n_outputs; i++) {
		for (int j=0; j<n_features; j++) {
#pragma HLS PIPELINE
			amem[i][j] = a[i*n_features + j];
		}
		bmem[i] = b[i];
	}

#pragma HLS dataflow

	read(x, xbuf, iter);
	compute(xbuf, amem, bmem, obuf, iter);
	write(obuf, output, iter);

  }

#include "cf_stub.h"
void _p0_LinReg_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen);
void _p0_LinReg_1_noasync(data_t * x, coef_t a[320], coef_t b[10], data_t * output, len_t datalen)
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










