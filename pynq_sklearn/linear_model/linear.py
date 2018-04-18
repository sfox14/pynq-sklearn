"""
Scikit-Learn estimator for PYNQ + SDSoC Accelerators
	eg. PynqLinearRegression()
"""
import os
import inspect
import numpy as np
import time

from ..base import PynqMixin
from sklearn.linear_model import LinearRegression

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
BIT_DIR = os.path.join(ROOT_DIR, "..", "bitstreams")
LIB_DIR = os.path.join(ROOT_DIR, "..", "libraries")


class PynqLinearRegression(PynqMixin, LinearRegression):
	"""
		Ordinary least squares Linear Regression. Includes the option of
		offloading predict() to programmable logic on the PYNQ-Z1
		Parameters
		----------
		hw_accel: boolean, optional, default True
			Whether to offload predict() to FPGA. If False, computation
			is performed on the ARM Cortex-A9 processor
		**kwargs: Parameters of sklearn.linear_model.LinearRegression()
		Attributes
		----------
		coef_: array, shape (n_features, ) or (n_targets, n_features)
			Estimated coefficients for the linear regression problem. Our
			hardware accelerator is limited to problems with
			n_features=13 and n_targets=1.
			*** FUTURE WORK=Software configurable FPGA Overlay ***
		intercept_: array
			Independent term in the linear model
	"""

	def __init__(self, hw_accel=True, **kwargs):

		""" set attributes automatically """
		args, _, _, values = inspect.getargvalues(inspect.currentframe())
		values.pop("self")
		for arg, val in values.items():
			setattr(self, arg, val)
		# print("{} = {}".format(arg, val))

		""" properties of fixed hw accelerator """
		# hwargs = {"bitstream": os.path.join(BIT_DIR, "linear_simple.bit"),
		#          "library": os.path.join(LIB_DIR, "liblreg_simple.so")}
		#hwargs = {"bitstream": os.path.join(BIT_DIR, "linear_sg.bit"),
		#		  "library": os.path.join(LIB_DIR, "liblreg_sg.so")}
		hwargs = {"bitstream": os.path.join(BIT_DIR, "linear_32104.bit"),
				  "library": os.path.join(LIB_DIR, "liblreg32104.so")}
		self.input_width = 32 #13
		self.output_width = 10 #1

		""" multiple inheritance """
		super(PynqLinearRegression, self).__init__(**hwargs, **kwargs)

	def fit(self, x, y):
		""" Default SW fit() """
		super(PynqLinearRegression, self).fit(x, y)

		""" HW post processing """
		if self.hw_accel:
			if self.check_array(x, y) is not True:
				raise ValueError("HW Accelerator does not fit the input data")

		self.coef_ = (self.coef_ * (2 ** 20)).astype("int32")
		self.intercept_ = (self.intercept_ * (2 ** 20)).astype("int32")

		return

	def predict(self, x):
		if self.hw_accel:
			datalen = len(x)
			# HW is limited to batch size = 1000
			if datalen > 1000:
				raise RuntimeError("Batch size exceeds HW accelerator")
			a, b, din, dout = self.preprocess(x, len(x))
			out = self.run(a, b, din, dout, len(x))
			return np.array(out).reshape(-1, self.output_width)

		else:
			return super(PynqLinearRegression, self).predict(x)

	def check_array(self, x, y):
		assert len(x) == len(y)
		dlen = len(x)
		if len(x.flatten()) != dlen * self.input_width or len(y.flatten()) != \
				dlen * self.output_width:
			return False
		else:
			return True

	@property
	def ffi_interface(self):
		# return """ void _p0_LinReg_1_noasync(float x[13000], float a[13],
		# float b, float output[1000], int datalen); """
		return """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """

	def config(self):
		pass

	def preprocess(self, x, bsize):
		a, b, din, dout = self.allocate_mem(bsize)
		# self.coef_
		coef_hw = self.coef_.flatten().astype(np.int32)
		for i in range(0, self.input_width*self.output_width):
			a[i] = coef_hw[i]
		# self.intercept_
		try:
			intc_hw = self.intercept_.flatten().astype(np.int32)
			for i in range(0, self.output_width):
				b[i] = intc_hw[i]
		except:
			b = float(self.intercept_)

		# input data
		x_hw = x.flatten().astype(np.int32)
		for i in range(bsize * self.input_width):
			din[i] = x_hw[i]
		return a, b, din, dout

	def allocate_mem(self, bsize):
		# self.coef_
		a = self.mem_init(self.input_width * self.output_width * 4, "int")
		# self.intercept_
		b = self.mem_init(self.output_width * 4, "int")
		# input and output data
		din = self.mem_init(bsize * self.input_width * 4, "int")
		dout = self.mem_init(bsize * self.output_width * 4, "int")
		return a, b, din, dout

	def run(self, a, b, din, dout, dlen):
		print("Offloading predict to FPGA ...")

		if any("cdata" not in elem for elem in [str(din), str(a), str(b), str(dout)]):
			raise RuntimeError("Unknown buffer type!")
		self.interface._p0_LinReg_1_noasync(din, a, b, dout, dlen)
		out = []
		for i in range(self.output_width*dlen):
			out.append(dout[i])
		return out