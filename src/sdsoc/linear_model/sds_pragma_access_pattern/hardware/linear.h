
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;

#include "config.h"

// Class LinearRegression definition
template<class data_T, class coef_T>
class LinearRegression {
protected:
//  data_T temp_reg[width_out];
private:
public:
  void operator()(data_T x[width_in], coef_T a[width_out][width_in], coef_T b,
  	data_T output[width_out]);
  template<class data_TT, class coef_TT>
  friend ostream& operator<<(ostream & o, const LinearRegression<data_TT,coef_TT> &f);
};

//Main Linear Regression algorithm
template<class data_T, class coef_T>
void LinearRegression<data_T, coef_T>::operator()
	(data_T x[width_in], coef_T a[width_out][width_in], coef_T b,
		data_T output[width_out]) {

	int i, j;
	//data_t acc[width_out];

	for (i=0; i<width_out; i++) {
		data_t result = 0;
		for (j=0; j<width_in; j++) {
			data_t imm = x[j]*a[i][j];
			result += imm;
		}
		output[i]=result + b; //y_intercept
		//std::cout << output[i] << std::endl;
	}
}


template<class data_T, class coef_T>
ostream& operator<<(ostream& o, const LinearRegression<data_T, coef_T> &f) {
    for (int i = 0; i < (sizeof(f.shift_reg)/sizeof(data_T)); i++)
      {
        o << "shift_reg[" << i << "]= " << f.shift_reg[i] << endl;
      }
    o << "______________" << endl;
    return o;
}

// Function definition
#pragma SDS data access_pattern(x:SEQUENTIAL, a:SEQUENTIAL, output:SEQUENTIAL)
void LinReg(data_t x[bsize*width_in], coef_t a[width_out*width_in], coef_t b,
		data_t output[bsize*width_out], len_t datalen);
