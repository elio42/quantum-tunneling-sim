import matplotlib.pyplot as plt
import numpy as np

# Import the data from the file "out.txt"
data = np.loadtxt("out.txt")


plt.plot(np.arange(len(data)), data, label="Data")
plt.xlabel("Index")
plt.ylabel("Value")
plt.title("Plot of Data from out.txt")
plt.legend()
plt.show()