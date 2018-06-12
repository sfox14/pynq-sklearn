"""
Scikit-Learn estimator for PYNQ + SDSoC Accelerators
	eg. PynqLogisticRegression()
"""
import os
import inspect
import numpy as np
import time
import ctypes

from pynq.xlnk import ContiguousArray
from ..base import PynqMixin
from ..register import HybridLibrary
from sklearn.linear_model import LogisticRegression

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
BIT_DIR = os.path.join(ROOT_DIR, "bitstreams")
LIB_DIR = os.path.join(ROOT_DIR, "libs")
BIT_WIDTH = 32
FRAC_WIDTH = 20
MAX_OUT = 10000


# noinspection PyInterpreter
class PynqLogisticRegression(PynqMixin, LogisticRegression):
	"""
		Logistic Regression (aka logit, MaxEnt) classifier. Includes the
		option of offloading decision_function() to programmable logic on the
		PYNQ-Z1
		Parameters
		----------
		hw_accel: boolean, optional, default True
			Whether to offload predict() to FPGA. If False, computation
			is performed on the ARM Cortex-A9 processor
		**kwargs: Parameters of sklearn.linear_model.LogisticRegression()
		Attributes
		----------
		coef_: array, shape (1, n_features) or (n_classes, n_features)
			Estimated coefficients for the linear regression problem. Our
			hardware accelerator is limited to problems with
			n_features=32 and n_classes=10.
			*** FUTURE WORK=Software configurable FPGA Overlay ***
		intercept_: array, shape (n_classes,)
			Independent term in the linear model
	"""

	def __init__(self, hw=None, hw_accel=True, pipe_enable=False,
				 pipe_params=None, **kwargs):

		if hw is None or not isinstance(hw, HybridLibrary):
			raise AttributeError("argument 'hw' is required but not found")

		if not isinstance(hw, HybridLibrary):
			raise AttributeError("hw of type 'HybridLibrary' required but %s "
								 "found" %(type(hw)))

		""" set attributes """
		args, _, _, values = inspect.getargvalues(inspect.currentframe())
		values.pop("self")
		for arg, val in values.items():
			setattr(self, arg, val)
		# print("{} = {}".format(arg, val))

		""" properties of fixed hw accelerator """
		bitstream = os.path.join(BIT_DIR, self.hw.bitstream)
		library = os.path.join(LIB_DIR, self.hw.library)
		hwargs = {"bitstream": bitstream, "library": library}

		""" multiple inheritance """
		super(PynqLogisticRegression, self).__init__(**hwargs, **kwargs)

		self.n_features = self.hw.input_width
		self.n_classes = self.hw.output_width
		self.hw_accel = hw_accel

		# pipeline control variables
		self.pipe_params = pipe_params
		self.pipe_bypass = False
		#self.pipe_enable = pipe_enable
		

	def fit(self, x, y):
		""" Default SW fit() """
		super(PynqLogisticRegression, self).fit(x, y)

		""" HW post processing """
		# copy to xlnk cma buffer
		
		self.coef_hw = self.copy_array(self.coef_ * (1<<FRAC_WIDTH),
									   dtype=np.int32)
		
		self.intercept_hw = self.copy_array((self.intercept_*(
				1<<FRAC_WIDTH)).reshape(-1,1), dtype=np.int32)

		# allocate outBuffer
		self.outBuffer = self.xlnk.cma_array(shape=(MAX_OUT, self.n_classes),
											 dtype=np.int32)
		return

	def decision_function(self, x):
		if self.hw_accel:

			if self.pipe_bypass:
				return x

			datalen = len(x)
			if datalen > MAX_OUT:
				raise RuntimeError("Buffer overflow: outBuffer required "
								   "exceeds MAX_OUT")
			if isinstance(x, ContiguousArray) and x.pointer:
				inBuffer = x.pointer
			else:
				if not self.hw.dma_sg:
					raise RuntimeError("Contiguous array required")
				xin = x.flatten()
				if xin.dtype != np.int32:
					# convert to fixed point
					xin = (xin*(1<<FRAC_WIDTH)).astype(np.int32)
				inBuffer = self._ffi.cast("int *", xin.ctypes.data)

			if self.hw.pipe:
				self.pipeline(inBuffer, self.pipeBuffer.pointer, datalen)
				return self.pipeBuffer[:datalen]
			else:
				self.run(self.coef_hw.pointer, self.intercept_hw.pointer,
						 inBuffer, self.outBuffer.pointer, datalen)

			return self.outBuffer[:datalen]
			
			# Make sure we return a ContiguousArray with pointer
			#view = out.view(ContiguousArray)
			#view.pointer = self.outBuffer.pointer
			#view.return_to = None
			#return view
			
		else:
			return super(PynqLogisticRegression, self).decision_function(x)

	@property
	def ffi_interface(self):
		return """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); 
		void _p0_Pipe_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """


	def run(self, a, b, din, dout, dlen):
		if any("cdata" not in elem for elem in [str(din), str(a), str(b),
												str(dout)]):
			raise RuntimeError("Unknown buffer type!")
		self.interface._p0_LinReg_1_noasync(din, a, b, dout, dlen)


	def pipeline(self, din, dout, dlen):
		if any("cdata" not in elem for elem in [str(din),
												str(self.pipe_params["a"]),
												str(self.pipe_params["b"]),
												str(dout)]):
			raise RuntimeError("Unknown buffer type!")
		self.interface._p0_Pipe_1_noasync(din, self.pipe_params["a"],
										  self.pipe_params["b"], dout, dlen)


	# We override "set_params" to configure and implement a hardware
	# pipeline using scikit-learn's Pipeline class. This function can be
	# used as a template, and copied for any PYNQ scikit-learn estimator.
	def set_params(self, **params):
		super(self.__class__, self).set_params(**params)

		self.pipe_bypass = False
		# configure the pipeline
		if self.hw.pipe:
			# configure the outBuffer of the pipeline
			if self.pipe_params is not None:
				n_out = self.pipe_params["n_out"]
				self.pipeBuffer = self.xlnk.cma_array(shape=(MAX_OUT, n_out),
													  dtype=np.int32)
				print("stage1: 	", self.__class__.__name__)
			else:
				self.pipe_bypass = True
				print("sw bypass: 	", self.__class__.__name__)

