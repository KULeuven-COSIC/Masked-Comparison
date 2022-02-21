import chipwhisperer as cw
import matplotlib.pyplot as plt
from tqdm import tqdm
import numpy as np
import time
from scipy import stats

##USER DEFINED PARAMETERS
max_trace_length = 500

#Iterative processing of each trace
def second_order(trace, n, m, c2, c3, c4):
    delta1 = np.zeros((len(trace),len(trace)))
    delta2 = np.zeros((len(trace), len(trace)))
    for i in range(len(trace)):
        temp = trace[i] - m[i]
        for j in range(len(trace)):
            delta1[i, j] = temp

    delta2 = np.transpose(delta1)

            # c4[i, j] = c4[i, j] - 2 * c3[0, i, j] * delta2 / n - 2 * c3[1, i, j] * delta1 / n + c2[0, i, j] \
            #           * (delta2 ** 2) / (n ** 2) + 4 * c2[1, i, j] * delta1 * delta2 / (n ** 2) + c2[2, i, j] \
             #          * (delta1 ** 2) / (n ** 2) + (delta1 ** 2) * (delta2 ** 2) * (
              #                     n ** 3 - 4 * n ** 2 + 6 * n - 3) / n ** 3
    c4[:,:] = c4 - 2 * c3[0, :, :] * delta2 / n - 2 * c3[1, :, :] * delta1 / n + c2[0, :, :] \
    * (delta2 ** 2) / (n ** 2) + 4 * c2[1, :, :] * delta1 * delta2 / (n ** 2) + c2[2, :, :] \
    * (delta1 ** 2) / (n ** 2) + (delta1 ** 2) * (delta2 ** 2) * ( \
                n ** 3 - 4 * n ** 2 + 6 * n - 3) / n ** 3
    #c3[1, i, j] = c3[1, i, j] - 2 * c2[1, i, j] * delta2 / n - c2[2, i, j] * delta1 / n \
     #             + delta1 * (delta2 ** 2) * (n ** 2 - 3 * n + 2) / n ** 2
    #c3[0, i, j] = c3[0, i, j] - 2 * c2[1, i, j] * delta1 / n - c2[0, i, j] * delta1 / n \
     #             + delta1 ** 2 * delta2 * (n ** 2 - 3 * n + 2) / n ** 2
    c3[1, :,:] = c3[1, :,:] - 2 * c2[1, :,:] * delta2 / n - c2[2, :,:] * delta1 / n \
                  + delta1 * (delta2 ** 2) * (n ** 2 - 3 * n + 2) / n ** 2
    c3[0, :,:] = c3[0, :,:] - 2 * c2[1, :,:] * delta1 / n - c2[0, :,:] * delta1 / n \
                  + delta1 ** 2 * delta2 * (n ** 2 - 3 * n + 2) / n ** 2
    # c2[2, i, j] = c2[2, i, j] + (delta2 ** 2) * (n - 1) / n
    # c2[1, i, j] = c2[1, i, j] + delta2 * delta1 * (n - 1) / n
    # c2[0, i, j] = c2[0, i, j] + (delta1 ** 2) * (n - 1) / n
    c2[2, :, :] = c2[2, :, :] + (delta2 ** 2) * (n - 1) / n
    c2[1, :, :] = c2[1, :, :] + delta2 * delta1 * (n - 1) / n
    c2[0, :, :] = c2[0, :, :] + (delta1 ** 2) * (n - 1) / n
    for i in range(len(trace)):
        delta = trace[i] - m[i]
        m[i] = m[i] + delta / n

#Takes every n-th sample point from the input trace, reduces the number of sample points
def downsample(trace, n):
    new_trace = np.zeros(int(len(trace)/n))
    for i in range(int(len(trace)/n)):
        new_trace[i] = trace[i*n]
    return new_trace

fixedTraces = np.load(r'./Traces/fxTraces-0.npy')
downsample_factor = int(fixedTraces.shape[1]/max_trace_length)
sample_size = int(fixedTraces.shape[1]/downsample_factor)

#Parameters for second order t-Test
tArray = np.zeros((sample_size, sample_size))
m_fx = np.zeros(sample_size)
c2_fx = np.zeros((3, sample_size, sample_size))
c3_fx = np.zeros((2, sample_size, sample_size))
c4_fx = np.zeros((sample_size, sample_size))
m_rnd = np.zeros(sample_size)
c2_rnd = np.zeros((3, sample_size, sample_size))
c3_rnd = np.zeros((2, sample_size, sample_size))
c4_rnd = np.zeros((sample_size, sample_size))
cnt_fx = 0
cnt_rnd = 0

for multiple in tqdm(range(10)):
    fixedTraces = np.load(r'./Traces/fxTraces-{}.npy'.format(multiple))
    randomTraces = np.load(r'./Traces/rndTraces-{}.npy'.format(multiple))

    for i in tqdm(range(fixedTraces.shape[0])):
        cnt_fx += 1
        smallTrace = downsample(fixedTraces[i,:],downsample_factor)
        second_order(smallTrace, cnt_fx, m_fx, c2_fx, c3_fx, c4_fx)
    for i in tqdm(range(randomTraces.shape[0])):
        cnt_rnd += 1
        smallTrace = downsample(randomTraces[i, :], downsample_factor)
        second_order(smallTrace, cnt_rnd, m_rnd, c2_rnd, c3_rnd, c4_rnd)

mu_fx = c2_fx[1, :, :] / cnt_fx
var_fx = c4_fx[:, :] / cnt_fx - (mu_fx ** 2)
mu_rnd = c2_rnd[1, :, :] / cnt_rnd
var_rnd = c4_rnd[:, :] / cnt_rnd - (mu_rnd ** 2)
tArray = (mu_fx - mu_rnd) / np.sqrt((var_fx / cnt_fx) + (var_rnd / cnt_rnd))
np.save('tArray_generic.npy', tArray)

