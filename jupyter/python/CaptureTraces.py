import chipwhisperer as cw
import matplotlib.pyplot as plt
from tqdm import tqdm
import numpy as np
import time
from scipy import stats
import matplotlib as mpl
import random
import timeit

SCOPETYPE = 'OPENADC'
PLATFORM = 'CWLITEARM'
CRYPTO_TARGET = "NONE"


# CONNECT THE DEVICE AND FIND NUMSAMPLES AND DOWNSAMPLING RATE:
scope = cw.scope()
scope.default_setup()
target = cw.target(scope, cw.targets.SimpleSerial)

print(scope)
# setup scope parameters

scope.default_setup()

#print(scope)
prog = cw.programmers.STM32FProgrammer
fw_path = './src/simpleserial-hoComp-{}.hex'.format(PLATFORM)
print(fw_path)

cw.program_target(scope, prog, fw_path,baud=38400)
time.sleep(5)
#Setup the numSamples parameter

ktp = cw.ktp.Basic()  # object to generate fixed/random key and text (default fixed key, random text)
key, text = ktp.next()  # get our fixed key and random text

scope.adc.timeout = 2
ret = cw.capture_trace(scope, target, text, key)

while (scope.adc.trig_count > 2000000):
    ret = cw.capture_trace(scope, target, text, key)
print(scope.adc.trig_count)
if ret:
    trace = ret

scope.adc.decimate = round(scope.adc.trig_count / 24400 + 0.5)
numPoints = round(scope.adc.trig_count / scope.adc.decimate)
scope.adc.samples = numPoints
print(numPoints)

# CAPTURE AND SAVE THE TRACES
ktp = cw.ktp.Basic()
ktp.fixed_key = False
ktp.fixed_text = False
numLoops = 10
N = 10000

usedKeys = np.zeros(N)
usedTimes = np.zeros(N)

for j in range(numLoops):
    fixedTraces = [[] for i in range(0)]
    randomTraces = [[] for i in range(0)]

    for i in tqdm(range(N), desc='Capturing traces'):

        successful = False
        while not successful:
            key, text = ktp.next()  # manual creation of a key, text pair can be substituted here
            ret = None
            while (not ret):
                ret = cw.capture_trace(scope, target, text, key)
                # ret = run_multi_trace(scope, target,text, key,0, 24400,8)
                # time.sleep(0.1)

            # print(ret)
            if (scope.adc.trig_count < 1600000):
                successful = True

            if successful:
                if (key[0] % 2 == 1):
                    fixedTraces.append(ret.wave[:])
                else:
                    randomTraces.append(ret.wave[:])

                usedKeys[i] = key[0] % 2
                usedTimes[i] = scope.adc.trig_count

            # TODO:save captured traces of length downsample*numPoints in a .npy file
            # time.sleep(0.1)


    outputString = './Traces/fxtraces-{}.npy'.format(j)
    np.save(outputString, fixedTraces)
    outputString = './Traces/rndtraces-{}.npy'.format(j)
    np.save(outputString, randomTraces)

scope.dis()
target.dis()
