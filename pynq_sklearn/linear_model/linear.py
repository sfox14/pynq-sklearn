"""
Scikit-Learn estimator for PYNQ + SDSoC Accelerators
	eg. PynqLinearRegression()
"""
import os
import inspect
import numpy as np
import time
import ctypes

from pynq.xlnk import ContiguousArray
from ..base import PynqMixin
from sklearn.linear_model import LinearRegression

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
BIT_DIR = os.path.join(ROOT_DIR, "..", "bitstreams")
LIB_DIR = os.path.join(ROOT_DIR, "..", "libraries")
BIT_WIDTH = 32
FRAC_WIDTH = 20
MAX_OUT = 10000


# noinspection PyInterpreter
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
		#self.bitstream = os.path.join(BIT_DIR, "linear_32_10_sg.bit")
		#self.library = os.path.join(LIB_DIR, "liblreg_32_10_sg.so")
		self.bitstream = os.path.join(BIT_DIR, "multi_sg.bit")
		self.library = os.path.join(LIB_DIR, "libmulti_sg.so")
		hwargs = {"bitstream": self.bitstream,
				  "library": self.library}
		
		""" multiple inheritance """
		super(PynqLinearRegression, self).__init__(**hwargs, **kwargs)
		
		self.n_features = 32
		self.n_targets = 10
		self.hw_accel = hw_accel
		

	def fit(self, x, y):
		""" Default SW fit() """
		super(PynqLinearRegression, self).fit(x, y)

		""" HW post processing """
		# copy to xlnk cma buffer
		
		self.coef_hw = self.copy_array(self.coef_ * (1<<FRAC_WIDTH),
									   dtype=np.int32)
		
		self.intercept_hw = self.copy_array((self.intercept_*(
				1<<FRAC_WIDTH)).reshape(-1,1),
									   dtype=np.int32)

		# allocate outBuffer
		self.outBuffer = self.xlnk.cma_array(shape=(MAX_OUT,self.n_targets),
											 dtype=np.int32)
		return

	def predict(self, x):
		if self.hw_accel:
		
			""" Two options for HW predict:
			#1.	Fast (AXI_DMA_SIMPLE)
			---------------------
			x: xlnk.cma_array()
			return:	xlnk.cma_array()
			
			#2. Slow (AXI_DMA_SG)
			-----------------
			x: np.array(shape=())
			return: xlnk.cma_array()
			
			** requires x.flatten()
			"""
			datalen = len(x)
			if datalen > MAX_OUT:
				raise RuntimeError("Buffer overflow: outBuffer required "
								   "exceeds MAX_OUT")
			if isinstance(x, ContiguousArray) and x.pointer:
				inBuffer = x.pointer
			else:
				if self.bitstream[-7:] != "_sg.bit":
					raise RuntimeError("Contiguous array required")
				xin = x.flatten()
				if xin.dtype != np.int32:
					# convert to fixed point
					xin = (xin*(1<<FRAC_WIDTH)).astype(np.int32)
				inBuffer = self._ffi.cast("int *", xin.ctypes.data)
			
			self.run(self.coef_hw.pointer, self.intercept_hw.pointer,
					 inBuffer, self.outBuffer.pointer, datalen)
			
			#return self.xlnk.cma_array(shape=(datalen, self.n_targets),
			#						   dtype=np.int32,
			#						   pointer=self.outBuffer.pointer)
			#return self.outBuffer[:datalen]
			#return self.outBuffer
			
			# Make sure we return a ContiguousArray with pointer
			view = self.outBuffer[:datalen].view(ContiguousArray)
			view.pointer = self.outBuffer.pointer
			view.return_to = None
			return view
			
		else:
			return super(PynqLinearRegression, self).predict(x)

	@property
	def ffi_interface(self):
		return """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """


	def run(self, a, b, din, dout, dlen):
		if any("cdata" not in elem for elem in [str(din), str(a), str(b),
												str(dout)]):
			raise RuntimeError("Unknown buffer type!")
		self.interface._p0_LinReg_1_noasync(din, a, b, dout, dlen)

