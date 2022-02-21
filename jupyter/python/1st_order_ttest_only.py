import chipwhisperer as cw
import matplotlib.pyplot as plt
from tqdm import tqdm
import numpy as np
import time
from scipy import stats

#PERFORM T-TEST AND SAVE MAX T-VALUE
fixedTraces = np.load(r'./Traces/fxTraces-0.npy')
numTimePoints = fixedTraces.shape[1]
sampleSizeFixed = 0
sampleSizeRandom = 0

meanFixed = np.zeros(numTimePoints)
meanRandom = np.zeros(numTimePoints)

ssquaredFixed = np.zeros(numTimePoints)
ssquaredRandom = np.zeros(numTimePoints)

for multiple in range(10):
    fixedTraces = np.load(r'./Traces/fxTraces-{}.npy'.format(multiple))
    randomTraces = np.load(r'./Traces/rndTraces-{}.npy'.format(multiple))

    sampleSizeFixed += fixedTraces.shape[0]
    sampleSizeRandom += randomTraces.shape[0]

print(sampleSizeFixed)

for multiple in tqdm(range(10)):
    fixedTraces = np.load(r'./Traces/fxTraces-{}.npy'.format(multiple))
    randomTraces = np.load(r'./Traces/rndTraces-{}.npy'.format(multiple))
    for trace in range(fixedTraces.shape[0]):
        meanFixed[:] += fixedTraces[trace, :]
    for trace in range(randomTraces.shape[0]):
        meanRandom[:] += randomTraces[trace, :]

meanFixed = meanFixed / sampleSizeFixed
meanRandom = meanRandom / sampleSizeRandom

for multiple in tqdm(range(10)):
    fixedTraces = np.load(r'./Traces/fxTraces-{}.npy'.format(multiple))
    randomTraces = np.load(r'./Traces/rndTraces-{}.npy'.format(multiple))
    for trace in range(fixedTraces.shape[0]):
        ssquaredFixed[:] += (fixedTraces[trace, :] - meanFixed[:]) ** 2
    for trace in range(randomTraces.shape[0]):
        ssquaredRandom[:] += (randomTraces[trace, :] - meanRandom[:]) ** 2

ssquaredFixed = ssquaredFixed / (sampleSizeFixed - 1)
ssquaredRandom = ssquaredRandom / (sampleSizeRandom - 1)

tArray = np.zeros(numTimePoints)
tArray[:] = (meanFixed[:]-meanRandom[:])/(ssquaredFixed[:]/sampleSizeFixed+ssquaredRandom[:]/sampleSizeRandom)**0.5

np.save('tArray_generic.npy', tArray)
