import matplotlib.pyplot as plt
import numpy as np


plt.rcParams.update({"text.usetex": True,"font.family": "serif","font.serif": ["Computer Modern Roman"],"figure.autolayout" : True})
tArray_in = np.load(r'tArray_generic.npy')

n_samples = tArray_in.shape[0]
tArray = np.zeros((int(n_samples/10)+1,int(n_samples/10)+1))

for i in range(int(n_samples/10)+1):

    for j in range(int(n_samples/10)+1):
        tArray[i,j] = np.max(tArray_in[10*i:10*(i+1),10*j:10*(j+1)])


textwidth = 13.64792;
textwidth_inches = textwidth/2.54
plt.figure(figsize=(textwidth_inches, textwidth_inches/1.5))
plt.tick_params(axis='x', labelsize=10)
plt.tick_params(axis='y', labelsize=10)
plt.xlabel('Samples x$10$',size=10)
plt.ylabel('Samples x$10$',size=10)

plt.pcolor(abs(tArray), cmap='bwr', vmin=0, vmax=13)
plt.colorbar(extend='max').set_label('$t$-statistic', rotation=270)


#plt.grid()
plt.savefig('2dplot_generic.pdf', format='pdf')
plt.show()
#plt.subplots_adjust(bottom=0.5)
#print(numLeakagePoints)
print(np.argwhere(abs(tArray) >= 4.5))
print(tArray[np.argwhere(abs(tArray) >= 4.5)])
print(np.max(tArray))