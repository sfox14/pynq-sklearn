from pynq import Overlay, PL, Xlnk
import numpy as np
import abc
import cffi


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

    def mem_init(self, memSize=1024, dtype="int"):
        """ Allocate CMA buffer """
        buf = self.xlnk.cma_alloc(memSize)
        return self.xlnk.cma_cast(buf, dtype)

    @abc.abstractmethod
    def ffi_interface(self):
        pass
    
    @abc.abstractmethod
    def config(self):
        """ reconfigure the overlay at runtime
         eg. adjust state machine, flip switches, load hw parameters
                    *** FUTURE WORK ***
        """
        pass

    @abc.abstractmethod
    def preprocess(self):
        """ preprocess numpy arrays and allocate CMA buffers """
        pass

    @abc.abstractmethod
    def run(self):
        """ call interface library """
        pass
    