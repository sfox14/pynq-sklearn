
#include "pipe.h"


void LinearRegression(fifoA_t &rbuf, coef_t amem[n_outputs][n_components], coef_t bmem[n_outputs],
		fifoB_t &obuf, len_t datalen)
{
	static op_t result[n_outputs];
	static parA_t xdat[n_outputs];

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
macc:	for (int i=0; i<(n_components/rp_width); i++) {
#pragma HLS PIPELINE

			parA_t xdin;
			rbuf >> xdin;

			for (int j=0; j<n_outputs; j++) {
#pragma HLS UNROLL
#pragma HLS PIPELINE

				xdat[j] = xdin;
				parA_t xval = xdat[j];

				for (int k=0; k<rp_width; k++) {
#pragma HLS UNROLL
#pragma HLS PIPELINE
					data_t xval_uint = xval((bit_width-1) + k*(bit_width), k*(bit_width));
					coef_t aval_uint = amem[j][i*rp_width + k];
					op_t op1, op2;
					op1.V = xval_uint.range(bit_width-1,0);
					op2.V = aval_uint.range(bit_width-1,0);
					//imm[j][k][0] = op1*op2;
					result[j] += op1*op2;
				}
			}
		}

		// result
res:	for (int ii=0; ii<n_outputs; ii++) {
#pragma HLS PIPELINE
			coef_t bval_uint = bmem[ii];
			op_t bop, imm0;
			bop.V = bval_uint.range(bit_width-1,0);
			imm0 = result[ii] + bop;
			parB_t imm1;
			imm1.range(31,0) = imm0.V;
			obuf << imm1;
		}
	}
}








