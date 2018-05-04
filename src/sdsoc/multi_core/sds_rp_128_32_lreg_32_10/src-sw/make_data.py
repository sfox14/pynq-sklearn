import numpy as np

N = 10
n_features = 32
n_outputs = 10

np.random.seed(23)

x = np.random.rand(N*n_features).astype(np.float32)*2 - 1
a = np.random.rand(n_features*n_outputs).astype(np.float32)*2 - 1
b = np.random.rand(n_outputs).astype(np.float32)*2 - 1

print(x,a,b)

with open("x_rand.dat", "w") as f:
	for dat in x:
		f.write("%f\n"%(dat))

with open("a_rand.dat", "w") as f:
	for dat in a:
		f.write("%f\n"%(dat))

with open("b_rand.dat", "w") as f:
	for dat in b:
		f.write("%f\n"%(dat))


X = x.reshape(-1, n_features)
A = a.reshape(-1, n_features)

print( (np.dot(X, A.T)+ b)[0] )