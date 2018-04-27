"""
Scikit-Learn estimator for PYNQ + SDSoC Accelerators
	eg. PynqLinearRegression()
"""
import os
import inspect
import numpy as np
import time
import ctypes

from ..base import PynqMixin
from sklearn.linear_model import LinearRegression

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
BIT_DIR = os.path.join(ROOT_DIR, "..", "bitstreams")
LIB_DIR = os.path.join(ROOT_DIR, "..", "libraries")
BIT_WIDTH = 32
FRAC_WIDTH = 20
MAX_OUT = 1000


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
			n_features=32 and n_targets=10.
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
		hwargs = {"bitstream": os.path.join(BIT_DIR, "linear_32_10_sg.bit"),
				  "library": os.path.join(LIB_DIR, "liblreg_32_10_sg.so")}
		self.input_width = 32
		self.output_width = 10

		""" multiple inheritance """
		super(PynqLinearRegression, self).__init__(**hwargs, **kwargs)

	def fit(self, x, y):
		""" Default SW fit() """
		super(PynqLinearRegression, self).fit(x, y)

		""" HW post processing """
		if self.hw_accel:
			if self.check_array(x, y) is not True:
				raise ValueError("HW Accelerator does not fit the input data")

		# copy to xlnk cma buffer
		self.coef_hw = self.copy_array(self.coef_ * (1<<FRAC_WIDTH),
									   dtype=np.int32)
		self.intercept_hw = self.copy_array(self.intercept_*(1<<FRAC_WIDTH),
									   dtype=np.int32)

		# allocate outBuffer
		self.outBuffer = self.xlnk.cma_array(shape=(MAX_OUT * 10),
											 dtype=np.int32)
		return

	def predict(self, x):
		if self.hw_accel:
			datalen = int( len(x)/self.input_width )
			# HW is limited to batch size = 1000
			if datalen > 1000:
				raise RuntimeError("Batch size exceeds HW accelerator")
			self.run(self.coef_hw.pointer, self.intercept_hw.pointer,
					 x.pointer, self.outBuffer.pointer, datalen)
			return self.outBuffer[:datalen * self.output_width].reshape(-1,
															self.output_width)
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
		return """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """

	def config(self):
		pass

	def preprocess(self):
		pass

	def run(self, a, b, din, dout, dlen):
		if any("cdata" not in elem for elem in [str(din), str(a), str(b),
												str(dout)]):
			raise RuntimeError("Unknown buffer type!")
		self.interface._p0_LinReg_1_noasync(din, a, b, dout, dlen)

