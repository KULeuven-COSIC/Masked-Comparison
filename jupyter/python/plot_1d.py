import matplotlib.pyplot as plt
import numpy as np


plt.rcParams.update({"text.usetex": True,"font.family": "serif","font.serif": ["Computer Modern Roman"],"figure.autolayout" : True})
tArray = np.load(r'tArray_generic.npy')

textwidth = 13.64792;
textwidth_inches = textwidth/2.54
plt.figure(figsize=(textwidth_inches, textwidth_inches/3.5))
plt.tick_params(axis='x', labelsize=10)
plt.tick_params(axis='y', labelsize=10)
plt.xlabel('Samples',size=10)
plt.ylabel('t-statistic',size=10)
plt.plot(tArray[:])
plt.axhline(y=4.5,color='r')
plt.axhline(y=-4.5,color='r')
#plt.grid()
plt.savefig('1dplot_generic.pdf', format='pdf')
plt.show()
#plt.subplots_adjust(bottom=0.5)
#print(numLeakagePoints)
print(np.argwhere(abs(tArray) >= 4.5))
print(tArray[np.argwhere(abs(tArray) >= 4.5)])