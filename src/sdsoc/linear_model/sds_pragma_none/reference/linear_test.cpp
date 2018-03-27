
#include "config.h"
#include "linear.h"


static void
LinReg_sw(data_t x[bsize*width_in], coef_t a[width_out*width_in], coef_t b,
        data_t output[bsize*width_out], len_t datalen)
  {

    data_t xbuf[bsize][width_in], abuf[width_out][width_in], obuf[bsize][width_out];

    for (int i=0; i<bsize; i++) {
        for (int j=0; j<width_in; j++) {
            xbuf[i][j] = x[(i*width_in) + j];
        }
        for (int k=0; k<width_out; k++) {
            obuf[i][k] = x[(i*width_in) + k];
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

int main()
{

	coef_t a[width_out*width_in];
    coef_t b;
    data_t x[bsize*width_in];
    data_t output[bsize*width_out], output_sw[bsize*width_out];
    len_t datalen = 50;

    //create input data
    for (int i = 0; i < bsize; i++) {
        for (int j=0; j<width_in; j++) {
        	if (i < datalen) {
        		x[(i*width_in) + j]=i+j;
        	} else{
        		x[(i*width_in) + j]=0; // fill the cma buffer
        	}
        }
    }
    //create weights
    for (int i = 0; i<width_out; i++) {
        for (int j=0; j<width_in; j++) {
            a[(i*width_in) + j]=i+j;
        }
    }

    b = 0;

    std::cout << x[0] << ";" << x[1] << ";" << x[10] << ";" << a[10] << std::endl;

    LinReg(x, a, b, output, datalen);
    LinReg_sw(x, a, b, output_sw, datalen);

    for (int i = 0; i < datalen; i++) {
        for (int j = 0; j < width_out; j++) {

            if ( i<= 20 ) {
                std::cout << output[(i*width_out) + j] << ";" << output_sw[(i*width_out) + j] << std::endl;
            }
            if (output[(i*width_out) + j] != output_sw[(i*width_out) + j]) {
                std::cout   << "Mismatch: LinReg(" << i << ") = "
                            << output[(i*width_out) + j] << ";  LinReg_sw(" << i << ") = "
                            << output_sw[(i*width_out) + j] << std::endl;
                std::cout << "Test failed" << std::endl;
                return -1;
            }
        }

    }
    std::cout << "Test passed" << std::endl;

    return 0;

  }
