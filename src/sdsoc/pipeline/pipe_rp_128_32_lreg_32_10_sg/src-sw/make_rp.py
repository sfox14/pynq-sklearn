import numpy as np

n_outputs = 32
FRAC_WIDTH = 20

np.random.seed(23)

seeds = np.random.randint(0, (2**32)-1, size=n_outputs, dtype="uint32")
theta = int(np.sqrt(1.0/(n_outputs))*(1<<FRAC_WIDTH))
thetas = (np.ones(n_outputs)*theta).astype(np.uint32)

with open("seeds%d.dat"%(n_outputs), "w") as f:
	for i, dat in enumerate(seeds):
		if i<(n_outputs-1):
			f.write("%d,\n"%(dat))
		else:
			f.write("%d\n"%(dat))

with open("thetas%d.dat"%(n_outputs), "w") as f:
	for i, dat in enumerate(thetas):
		if i<(n_outputs-1):
			f.write("%d,\n"%(dat))
		else:
			f.write("%d\n"%(dat))