from pynq import Overlay, PL, Xlnk
import numpy as np
import abc
import cffi

xlnk = Xlnk()


class PynqMixin(metaclass=abc.ABCMeta):
    """ Mixin class for all PYNQ estimators in scikit-learn """
    __metaclass__ = abc.ABCMeta

    def __init__(self, bitstream=None, library=None, load_overlay=True,
                    *args, **kwargs):
        super().__init__(*args, **kwargs)
        """ Init overlay and library """

        if PL.bitfile_name != bitstream and bitstream is not None:
            if load_overlay:
                Overlay(bitstream).download()
            else:
                raise RuntimeError("Incorrect Overlay loaded")
        self._ffi = cffi.FFI()
        self.interface = self._ffi.dlopen(library)
        self._ffi.cdef(self.ffi_interface)
        self.xlnk = Xlnk()

    def reset(self):
        """ Reset all CMA Buffers """
        xlnk = Xlnk()
        xlnk.xlnk_reset()
        return

    def copy_array(self, X, dtype=np.int32):
        """
        :param X: np.array
        :return: physically contiguous np.array
        """
        xin = X.flatten()
        dataBuffer = self.xlnk.cma_array(shape=len(xin), dtype=dtype)
        for i, data in enumerate(xin):
            dataBuffer[i] = data
        return dataBuffer


    @abc.abstractmethod
    def ffi_interface(self):
        pass


    @abc.abstractmethod
    def run(self, a, b, din, dout, dlen):
        """ call interface library """
        pass
    