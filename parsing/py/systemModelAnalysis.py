#!/usr/local/bin/python3
# Pouyan Keshavarzian
# EPFL STI IMT AQUA Dec 2020
#
import matplotlib.pyplot as plt
import csv
x=[]
y=[]
with open('dataIn/csv/rffModeling_vaExponentialSource_vaTFF_vaDFF_digitalOut.csv','r') as csvfile:
    next(csvfile)
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        x.append(float(row[0]))
        y.append(float(row[1]))
time  = [i * 1e9 for i in x ]
time  = [round(num, 0) for num in time]
print(*time, sep = "\n") 
length = len(time)
for i in range(length-1):
    numberOfMissedBits = (time[i+1]-time[i])/25.0
    print(numberOfMissedBits, sep = "\n")

zipstyle = [(t - s)/25.0 for s, t in zip(time, time[1:])]
print(zipstyle, sep = "\n")
import numpy as np
newlist = list(map(round, zipstyle))
print(newlist)
def bitgen(zipstyle, bitsOG):
    if(bitsOG):
        return  np.ones(zipstyle)
    else:
        return  np.zeros(zipstyle)
result=list(map(bitgen,newlist,y))

print(result, sep = "\n")

with open('outputs/vaSimDataParsed.txt', 'w') as f:
    for item in result:
        f.write("%s\n" % item)

f.close()
