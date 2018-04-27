"""
Scikit-Learn estimator for PYNQ + SDSoC Accelerators
	eg. PynqBinaryRandomProjection()
"""
import os
import inspect
import numpy as np
import time
import ctypes

from ..base import PynqMixin
from sklearn.random_projection import SparseRandomProjection

ROOT_DIR = os.path.dirname(os.path.realpath(__file__))
BIT_DIR = os.path.join(ROOT_DIR, "..", "bitstreams")
LIB_DIR = os.path.join(ROOT_DIR, "..", "libraries")
BIT_WIDTH = 32
FRAC_WIDTH = 20
MAX_OUT = 10000


""" Random seeds are hard coded in programmable logic """
seeds = [2221777491, 2873750246, 4067173416, 794519497, 3287624630, 3357287912,
		 1212880927, 2464917741,949382604, 1898004827, 2947301415, 790253146,
		 717857433, 428724147, 1685527558, 943400827, 2654514605,
		 3579164172, 1769226289, 1832943298, 10586571, 3240865749,
		 3796889285,  4281504960, 3800820748, 356885, 1290249776,
		 4191694121, 2532234831, 1800111962, 4202311614, 2927266873]

def binary_random_matrix(n_features=128, n_components=32):
	# initialise random matrix
	rij = np.zeros(shape=(n_features, n_components))

	# make sure that mem matches seeds
	if n_components != len(seeds):
		print("Error: n_components must equal len(seeds)")

	# initialise lfsrs - pseudo random number generators
	lfsrs = [LFSR(seeds[i]) for i in range(len(seeds))]

	# create random matrix
	for j in range(n_components):
		lfsr = lfsrs[
			j]  # each dimension j represents a unique n_component and LFSR
		for i in range(n_features):
			rnd = lfsr.next()
			rnd = rnd * 2 - 1
			rij[i][j] = rnd
		lfsr.reset()

	return rij.T*(np.sqrt(1.0/n_components))

class LFSR(object):
	"""
		Software version of a linear feedback shift-register (LFSR)
	"""

	def __init__(self, seed, taps=[2, 7, 22, 32]):
		self.seed = seed
		self.taps = taps
		self.state = '{0:032b}'.format(self.seed)[::-1] #bitstring

	def next(self):
		xor = 0
		for t in self.taps:
			xor += int(self.state[32-t])
		if xor%2 == 0.0:
			xor = 0
		else:
			xor = 1
		self.state = str(xor) + self.state[:-1]
		return xor

	def reset(self):
		self.state = '{0:032b}'.format(self.seed)[::-1]


class PynqBinaryRandomProjection(PynqMixin, SparseRandomProjection):
	"""
		Dimensionality reduction through binary random projection.

		A Binary Random Projection is one case of Sparse Random Projection
		where the density parameter equals 1, i.e. s=1. This class
		offloads the computation, i.e. transform(), to programmable logic
		on the PYNQ-Z1.

		From scikit-learn (random_projection.py):
		If we note `s = 1 / density` the components of the random matrix are
		drawn from:
		  - -sqrt(s) / sqrt(n_components)   with probability 1 / 2s
		  -  0                              with probability 1 - 1 / s
		  - +sqrt(s) / sqrt(n_components)   with probability 1 / 2s

		Parameters
		----------
		hw_accel: boolean, optional, default True
			Whether to offload predict() to FPGA. If False, computation
			is performed on the ARM Cortex-A9 processor
		**kwargs: Parameters of sklearn.random_projection.SparseRandomProjection()

		Attributes
		----------
		components_ : Matrix with shape [n_components, n_features]
			Random binary matrix used for projection (replication of hardware)
	"""

	def __init__(self, **kwargs):

		""" set attributes """
		args, _, _, values = inspect.getargvalues(inspect.currentframe())
		values.pop("self")
		for arg, val in values.items():
			setattr(self, arg, val)

		""" properties of fixed hw accelerator """
		hwargs = {"bitstream": os.path.join(BIT_DIR, "rp_128_32.bit"),
				  "library": os.path.join(LIB_DIR, "librp_128_32.so")}

		""" multiple inheritance """
		super(PynqBinaryRandomProjection, self).__init__(**hwargs, **kwargs)

		self.n_features = 128
		self.n_components = 32
		self.hw_accel = True


	def fit(self, X, y=None):
		""" Generate a binary random projection matrix which is equivalent to
		the random matrix implemented using LFSRs in programmable logic """

		self.components_ = self._make_random_matrix(self.n_features, self.n_components)

		if self.hw_accel:
			if self.check_array(X, y) is not True:
				raise ValueError("HW Accelerator does not fit the input data")

		""" HW post processing """
		# allocate outBuffer
		self.outBuffer = self.xlnk.cma_array(shape=(MAX_OUT * 10),
											 dtype=np.int32)
		return self

	def transform(self, x):
		if self.hw_accel:
			datalen = int( len(x)/self.n_features )
			# HW is limited to batch size = 1000
			if datalen > MAX_OUT:
				raise RuntimeError("Batch size exceeds HW accelerator")
			self.run(x.pointer, self.outBuffer.pointer, datalen)
			return self.outBuffer[:datalen * self.n_components].reshape(-1,
															self.n_components)
		else:
			return super(PynqBinaryRandomProjection, self).transform(x)

	def _make_random_matrix(self, n_components, n_features):
		return binary_random_matrix(n_components, n_features)

	def check_array(self, x, y):
		assert len(x) == len(y)
		dlen = len(x)
		if len(x.flatten()) != dlen * self.n_features or len(y.flatten()) != \
				dlen * self.n_components:
			return False
		else:
			return True

	@property
	def ffi_interface(self):
		return """ void _p0_RandomProjection_1_noasync(int *x, int *output, int datalen); """


	def run(self, din, dout, dlen):
		if any("cdata" not in elem for elem in [str(din), str(dout)]):
			raise RuntimeError("Unknown buffer type!")
		self.interface._p0_RandomProjection_1_noasync(din, dout, dlen)

