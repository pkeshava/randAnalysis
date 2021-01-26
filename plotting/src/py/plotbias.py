"""
docstring
"""
import matplotlib.pyplot as plt
import numpy as np
vt=np.array([0.200, 0.225, 0.250, 0.275, 0.300, 0.325, 0.350, 0.375, 0.400, 0.425, 0.450, 0.475, 0.499])
expectedbias = np.loadtxt('dataIn/txt/expectedbias.txt', delimiter=',')
simulatedbias = np.loadtxt('dataIn/txt/simulatedbias.txt', delimiter=',')
fig = plt.figure()
ax1 = fig.add_subplot(111)
ax1.scatter(vt, expectedbias, s=10, c='b', marker="s", label='Model Prediction')
ax1.scatter(vt, simulatedbias, s=10, c='r', marker="o", label='Circuit Sim')

#ax1.xlabel('entry a')
#ax1.ylabel('entry b')
plt.legend(loc='lower left')
#plt.show()
import tikzplotlib
tikzplotlib.save("plotting/output/tex/parsedRNGdata/bias.tex")
