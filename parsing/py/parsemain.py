#!/usr/local/bin/python3
"""
high level support for doing this and that.
"""
# Pouyan Keshavarzian
# EPFL STI IMT AQUA Dec 2020

import csv
import numpy as np

class FillMissData:
    """
    high level support for doing this and that.
    """
    def __init__(self, filepointer):
        self.newlist=[]
        self.filepointer=filepointer
        self.time=[]
        self.data=[]
        self.num_miss_timestamp=[]

    def getdata(self):
        """
        docstring
        """
        with open(self.filepointer, 'r') as csvfile:
            next(csvfile)
            plots = csv.reader(csvfile, delimiter=',')
            for row in plots:
                self.time.append(float(row[0]))
                self.data.append(float(row[1]))
        #print('Original data:' , self.data)
        time_s = [i* 1e9 for i in self.time]
        time_s = [round(num,0) for num in time_s]
        # create iterable function for determining number of missed timesteps in data
        self.num_miss_timestamp = [(time_s[i+1]-time_s[i])/25.0 for i in
                range(len(time_s) - 1)]
        # or
        #self.num_miss_timestamp = [(t - s)/25.0 for s, t in zip(time_s, time_s[1:])]
        self.newlist = list(map(round, self.num_miss_timestamp))
        #print('Number of total timesteps per index: ', self.newlist)

def bitgen(num_miss_timestamp, original_bits):
    """
    docstring
    """
    if original_bits:
        return  np.ones(num_miss_timestamp)
    else:
        return  np.zeros(num_miss_timestamp)
def calcbias(array):
    """
    docstring
    """
    return (abs(len(array)/2.0-np.sum(array)))/len(array)


if __name__ == '__main__':
    #FP ="dataIn/csv/rffModeling_vaExponentialSource_vaTFF_vaDFF_digitalOut.csv"
    FP = "dataIn/csv/RBG_va_tb_only_DIGITALOUT.csv"
    dInst = FillMissData(FP)
    dInst.getdata()
    result=list(map(bitgen,dInst.newlist,dInst.data))
    resultcat = np.concatenate( result, axis=0)
    resultround = resultcat.round()
    resultcatbool=np.array(resultround, dtype=np.ubyte)
    #print('Concatenated bits: ', resultcatbool)
    resultcatpacked=np.packbits(resultcatbool)
    print(resultcatpacked)
    FPW="dataOut/parseCadenceSim/pythonParseCadenceSim.bin"
    resultcatpacked.tofile(FPW)
    resultimport = np.fromfile(FPW, dtype=np.ubyte)
    print('Array imported:', resultimport)
    bias=calcbias(resultcatbool)
    print('Bit bias from random bitstring is: ', bias)
# References
# zip iterator: https://realpython.com/python-zip-function/
# numpy bit packing: https://numpy.org/doc/stable/reference/generated/numpy.packbits.html
# convert list of arrays to pandas dataframe
# https://stackoverflow.com/questions/49540922/convert-list-of-arrays-to-pandas-dataframe
# numpy array to byte:
# https://numpy.org/doc/stable/reference/generated/numpy.ndarray.tobytes.html
# convert numpy array to 0 or 1 based on threshold:
# https://stackoverflow.com/questions/46214291/convert-numpy-array-to-0-or-1-based-on-threshold
# convert elements of an array to binary
# https://stackoverflow.com/questions/38935169/convert-elements-of-a-list-into-binary
