import matplotlib.pyplot as plt
import numpy as np

# Import the data from the file "output.csv" and remove the header
data = np.loadtxt("out1.csv", delimiter=",", skiprows=1)
data2 = np.loadtxt("out2.csv", delimiter=",", skiprows=1)
real = data[:, 0]
complex = data[:, 1]
real2 = data2[:, 0]
complex2 = data2[:, 1]

def niquistArr(n):
    #input ex 6 output [0, 1, 2, 3, -3, -2, -1]


    
    half = n // 2
    a = np.concatenate((np.arange(0, half ), np.arange(-half, 0)))
    return a

size =1
plt.scatter(niquistArr(len(real//2)), real, label="Real Part", color='blue', s=size)
plt.scatter(niquistArr(len(real//2)), complex, label="Imaginary Part", color='red', s=size)

plt.scatter(niquistArr(len(real//2)), real2, label="Real Part 2", color='cyan', s=size)
plt.scatter(niquistArr(len(real//2)), complex2, label="Imaginary Part 2", color='magenta',s=size)


print(niquistArr(len(real)*2))






plt.xlabel("Index")
plt.ylabel("Value")
plt.title("Real and Imaginary Parts from output.csv")

plt.show()