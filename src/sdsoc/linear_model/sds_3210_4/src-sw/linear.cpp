
#include "linear.h"


void compute(fifoA_t &xbuf, coef_t amem[n_outputs][n_features], coef_t bmem[n_outputs],
		fifoB_t &obuf, len_t datalen)
{
	static data_t result[n_outputs];
	static parA_t xdat[n_outputs];
	//static data_t imm[n_outputs][4][3];

#pragma HLS array_partition variable=amem complete dim=1
#pragma HLS array_partition variable=result complete
#pragma HLS array_partition variable=xdat complete

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
macc:	for (int i=0; i<(n_features/4); i++) {
#pragma HLS PIPELINE

			parA_t xdin;
			xbuf >> xdin;

			for (int j=0; j<n_outputs; j++) {

				xdat[j] = xdin;
				parA_t xval = xdat[j];
				parA_t aval = 0;

				for (int k=0; k<4; k++) {
#pragma HLS UNROLL factor=4
#pragma HLS PIPELINE

					data_t op1 = xval((bit_width-1) + k*(bit_width), k*(bit_width));
					data_t op2 = amem[j][i*4 + k];
					//imm[j][k][0] = op1*op2;
					result[j] += op1*op2;
				}
/*
		adder: for (int col=1; col<3; col++) {
					// adder tree stages
#pragma HLS UNROLL
					add_op: for (int row=0; row < 4/(1<<col); row++) {
#pragma HLS UNROLL
						imm[j][row][col] = imm[j][2*row][col-1] + imm[j][2*row +1][col-1];
					}
				}
				result[j] += imm[j][0][2];
*/
			}
		}

		// result
res:	for (int ii=0; ii<n_outputs; ii++) {
#pragma HLS PIPELINE
			obuf << result[ii] + bmem[ii];
		}
	}

}

void read(data_t *x, fifoA_t &xbuf, len_t datalen) //[bsize*n_features]
{
	for (int i=0; i<datalen; i++) {
		for (int j=0; j<(n_features/4); j++) {
#pragma HLS PIPELINE
			parA_t temp;
			for (int k=0; k<4; k++) {
				int idx = j*4 + k;
				temp((bit_width-1) + k*(bit_width), k*(bit_width)) = x[i*n_features + idx];
			}
			xbuf << temp;
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








