
#include <stdint.h>
#include "sds_lib.h"
#include "config.h"
#include "linear.h"

class perf_counter
{
public:
     uint64_t tot, cnt, calls;
     perf_counter() : tot(0), cnt(0), calls(0) {};
     inline void reset() { tot = cnt = calls = 0; }
     inline void start() { cnt = sds_clock_counter(); calls++; };
     inline void stop() { tot += (sds_clock_counter() - cnt); };
     inline uint64_t avg_cpu_cycles() { return ((tot+(calls>>1)) / calls); };
};

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

	//coef_t a[width_out*width_in];
	coef_t *a;
	data_t *x, *output, *output_sw;
    coef_t b;
    //data_t x[bsize*width_in];
    //data_t output[bsize*width_out], output_sw[bsize*width_out];
    len_t datalen = 50;
    a = (coef_t *)sds_alloc(width_out*width_in*sizeof(coef_t));
    x = (data_t *)sds_alloc(bsize*width_in*sizeof(data_t));
    output = (data_t *)sds_alloc(bsize*width_out*sizeof(data_t));
    output_sw = (data_t *)malloc(bsize*width_out*sizeof(data_t));

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
    //for (int i = 0; i < width_out; i++) {
    //    b[i]=i;
    //}
    b = 0;

    std::cout << x[0] << ";" << x[1] << ";" << x[10] << ";" << a[10] << std::endl;

    perf_counter hw_ctr, sw_ctr;

    //hw_ctr.start();
    LinReg(x, a, b, output, datalen);
    //hw_ctr.stop();
    //sw_ctr.start();
    LinReg_sw(x, a, b, output_sw, datalen);
    //sw_ctr.stop();

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

    //uint64_t sw_cycles = sw_ctr.avg_cpu_cycles();
    //uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();
    //double speedup = (double) sw_cycles / (double) hw_cycles;

    //std::cout << "Average number of CPU cycles running LinReg in software: "
    //          << sw_cycles << std::endl;
    //std::cout << "Average number of CPU cycles running LinReg in hardware: "
    //          << hw_cycles << std::endl;
    //std::cout << "Speed up: " << speedup << std::endl;

    sds_free(a);
    sds_free(x);
    sds_free(output);
    free(output_sw);

    return 0;

  }
