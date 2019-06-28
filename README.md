# Scikit-Learn + PYNQ: A Software Library of FPGA/SoC Accelerators for Machine Learning

This repository provides a library of machine learning accelerators for Xilinx SoCs and FPGAs. We leverage both PYNQ and Scikit-Learn (two open source frameworks) and begin to create a software/hardware library of high performance applications. PYNQ lets users exploit performance benefits of programmable logic and microprocessors in Zynq SoCs while Scikit-Learn offers a rich ecosystem of tools and algorithms for data mining and data analysis.
 
## Quick Start:

Open a terminal on your PYNQ board and run:

```
sudo pip3.6 install git+https://github.com/sfox14/pynq-sklearn.git 
```

This will install the "pynq-sklearn" to your board, and will create the "sklearn" directory in PYNQ_JUPYTER_NOTEBOOKS. Here, you will find notebooks to test the repository.

**Note: Make sure you're using PYNQ v2.1 image**
