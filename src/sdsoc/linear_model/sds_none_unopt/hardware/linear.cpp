

#include "config.h"
#include "linear.h"

// Top-level function with class instantiated
void LinReg(data_t x[bsize*width_in], coef_t a[width_out*width_in], coef_t b,
		data_t output[bsize*width_out], len_t datalen)
  {

  	data_t xbuf[bsize][width_in], abuf[width_out][width_in], obuf[bsize][width_out];

  	for (int i=0; i<bsize; i++) {
  		for (int j=0; j<width_in; j++) {
  			xbuf[i][j] = x[(i*width_in) + j];
  		}
  		for (int k=0; k<width_out; k++) {
  			obuf[i][k] = 0;
  		}
  	}
  	for (int i=0; i<width_out; i++) {
  		for (int j=0; j<width_in; j++) {
  			abuf[i][j] = a[(i*width_in) + j];
  		}
  	}

    static LinearRegression<coef_t, data_t> lreg;
    for (int i=0; i<datalen; i++)
    	lreg(xbuf[i], abuf, b, obuf[i]);

    // make contiguous output
    for (int i=0; i<datalen; i++) {
    	for (int j=0; j<width_out; j++) {
    		output[(i*width_out) + j] = obuf[i][j];
    	}
    }


  }

#include "cf_stub.h"
extern "C" void _p0_Linneg_1_noasync(data_t x[2600], coef_t a[13], coef_t b, data_t output[200], len_t datalen);
extern "C" void _p0_LinReg_1_noasync(data_t x[2600], coef_t a[13], coef_t b, data_t output[200], len_t datalen)
{
  switch_to_next_partition(0);
  int start_seq[2];
  start_seq[0] = 200 * 4;
  start_seq[1] = 0;
  cf_request_handle_t _p0_swinst_LinReg_1_cmd;
  cf_send_i(&(_p0_swinst_LinReg_1.cmd_LinReg), start_seq, 2 * sizeof(int), &_p0_swinst_LinReg_1_cmd);
  cf_wait(_p0_swinst_LinReg_1_cmd);

  cf_send_i(&(_p0_swinst_LinReg_1.x_PORTA), x, 10400, &_p0_request_0);
  cf_send_i(&(_p0_swinst_LinReg_1.a_PORTA), a, 52, &_p0_request_1);
  cf_send_i(&(_p0_swinst_LinReg_1.b), &b, 4, &_p0_request_2);
  cf_send_i(&(_p0_swinst_LinReg_1.datalen), &datalen, 4, &_p0_request_4);

  cf_receive_i(&(_p0_swinst_LinReg_1.output_r_PORTA), output, 800, &_p0_LinReg_1_noasync_num_output_r_PORTA, &_p0_request_3);

  cf_wait(_p0_request_0);
  cf_wait(_p0_request_1);
  cf_wait(_p0_request_2);
  cf_wait(_p0_request_3);
  cf_wait(_p0_request_4);
}


