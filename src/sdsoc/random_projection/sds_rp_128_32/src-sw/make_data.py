import numpy as np

N = 10
n_features = 128
n_outputs = 32

np.random.seed(23)

x = np.random.rand(N*n_features).astype(np.float32)*2 - 1

with open("x_rand.dat", "w") as f:
	for dat in x:
		f.write("%f\n"%(dat))

X = x.reshape(-1, n_features)

print(X, X.shape)