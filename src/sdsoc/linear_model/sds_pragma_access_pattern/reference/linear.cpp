

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

  	//std::cout << xbuf[0][0] << ";" << xbuf[1][0] << ";" << xbuf[10][4] << ";" << a[10] << std::endl;

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
