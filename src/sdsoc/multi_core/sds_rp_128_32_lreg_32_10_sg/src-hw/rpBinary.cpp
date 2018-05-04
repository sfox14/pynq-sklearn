
#include "multi.h"

void BinaryRP(fifoA_t &xbuf, fifoB_t &obuf, len_t datalen)
{
	// seeds
	const unsigned int seeds[n_components] = {
		#include "seeds32.dat"
	};
	// scaling factors
	//const unsigned int thetas[n_outputs] = {
	//		#include "thetas32.dat"
	//};
	static data_t accum[n_components];
	static ap_uint<32> lfsr[n_components];

#pragma HLS array_partition variable=seeds complete
//#pragma HLS array_partition variable=thetas complete
#pragma HLS array_partition variable=accum complete
#pragma HLS array_partition variable=lfsr complete


	for (int i=0; i<datalen; i++) {

		//reset accumulator and LFSR

reset:	for (int k=0; k<n_components; k++) {
#pragma HLS UNROLL
			accum[k] = 0;
			lfsr[k] = seeds[k];
		}

rp:		for (int j=0; j<n_features; j++) {
#pragma HLS PIPELINE

			parA_t xin;
			parB_t xout;
			xbuf >> xin;

			for (int k=0; k<n_components; k++) {
#pragma HLS UNROLL
#pragma HLS PIPELINE

				bool b_32 = lfsr[k].get_bit(32-32);
				bool b_22 = lfsr[k].get_bit(32-22);
				bool b_7 = lfsr[k].get_bit(32-7);
				bool b_2 = lfsr[k].get_bit(32-2);
				bool new_bit = b_32 ^ b_22 ^ b_7 ^ b_2;

				lfsr[k] = lfsr[k] << 1;
				lfsr[k].set_bit(0, new_bit);

				if (new_bit) {
					accum[k] += xin;
				} else {
					accum[k] -= xin;
				}
			}
		}

scale:	for (int k=0; k<n_components; k++) {
#pragma HLS PIPELINE
			parB_t xout;
			op_t op1;
			op_t theta;
			op_t imm;
			op1.V = accum[k];
			theta.V = sfac;
			imm = op1*theta;
			xout.range(31,0) = imm.V;
			obuf << xout;
		}
	}


}






