"""
Hybrid Library Registry

    To add a new hybrid library:

    1. Create a new (key, value) -> (name, (param, value) entry to the
    RegistryDict below. If you don't want to add to the registy, you can pass
    a (param, value) dictionary as a parameter to your python class.

        hlib = {"param" : "value", ...}
        model = PynqLogisticRegression(hybrid_library=hlib)

    2. Each new entry must specify:
            bitstream - *.bit (path not required)
            lib - *.so (path not required)
            dma_sg - Boolean (Scatter-Gather DMA for input?)
            pipe - Boolean (Is the bitstream a hardware pipeline?)
            c_callable - string

    3. Additonal parameters should be added to help generalise your
    python class. For example: you might want to add a parameter called
    "x_shape" which asserts the bitstream's compatibility with different
    input shapes/sizes

"""

import numpy as np

RegistryDict = {

    "rp_multi": {
        # Required parameters:
        "bitstream": "multi.bit",
        "library": "libmulti.so",
        "dma_sg": False,
        "pipe": False,
        "c_callable": """ void _p0_RandomProjection_1_noasync(int *x, int *output, int datalen); """,
        "input_width": 128,
        "output_width": 32
    },

    "lr_multi": {
        # Required parameters:
        "bitstream": "multi.bit",
        "library": "libmulti.so",
        "dma_sg": False,
        "pipe": False,
        "c_callable": """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """,
        "input_width": 32,
        "output_width": 10
    },

    "rp_multi_sg": {
        # Required parameters:
        "bitstream": "multi_sg.bit",
        "library": "libmulti_sg.so",
        "dma_sg": True,
        "pipe": False,
        "c_callable": """ void _p0_RandomProjection_1_noasync(int *x, int *output, int datalen); """,
        "input_width": 128,
        "output_width": 32
    },

    "lr_multi_sg": {
        # Required parameters:
        "bitstream": "multi_sg.bit",
        "library": "libmulti_sg.so",
        "dma_sg": True,
        "pipe": False,
        "c_callable": """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """,
        "input_width": 32,
        "output_width": 10
    },

    "pipe": {
        # Required parameters:
        "bitstream": "pipe.bit",
        "library": "libpipe.so",
        "dma_sg": False,
        "pipe": True,
        "c_callable": """ void _p0_Pipe_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """,
        "input_width": 128,
        "output_width": 10
    },

    "pipe_sg": {
        # Required parameters:
        "bitstream": "pipe_sg.bit",
        "library": "libpipe_sg.so",
        "dma_sg": True,
        "pipe": True,
        "c_callable": """ void _p0_Pipe_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """,
        "input_width": 128,
        "output_width": 10
    },

    "rp_128_32": {
        # Required parameters:
        "bitstream": "rp_128_32.bit",
        "library": "librp_128_32.so",
        "dma_sg": False,
        "pipe": False,
        "c_callable": """ void _p0_RandomProjection_1_noasync(int *x, int *output, int datalen); """,
        "input_width": 128,
        "output_width": 32
    },

    "linear_32_10": {
        # Required parameters:
        "bitstream": "linear_32_10.bit",
        "library": "liblreg_32_10.so",
        "dma_sg": False,
        "pipe": False,
        "c_callable": """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """,
        "input_width": 32,
        "output_width": 10
    },

    "linear_32_10_sg": {
        # Required parameters:
        "bitstream": "linear_32_10_sg.bit",
        "library": "liblreg_32_10_sg.so",
        "dma_sg": True,
        "pipe": False,
        "c_callable": """ void _p0_LinReg_1_noasync(int *x, int a[320], int b[10], int *output, int datalen); """,
        "input_width": 32,
        "output_width": 10
    }

}

mandatory = ["bitstream", "library", "dma_sg", "pipe", "c_callable",
             "input_width", "output_width"]


class HybridLibrary:
    def __init__(self, **kwargs):
        for arg, val in kwargs.items():
            setattr(self, arg, val)
        Registry.check_lib(self)

    def __repr__(self):
        class_name = self.__class__.__name__
        attr = self.__dict__
        return '%s(): %s' % (class_name, str(attr))


class Registry:
    def __init__(self):
        pass

    @staticmethod
    def load(name):
        lib = HybridLibrary(**RegistryDict[name])
        Registry.check_lib(lib)
        return lib

    @staticmethod
    def check_lib(lib):
        for arg in mandatory:
            if not hasattr(lib, arg):
                raise AttributeError("HybridLibrary requires attribute: %s"
                                     %arg)

