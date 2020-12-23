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
        time_s = [i* 1e9 for i in self.time]
        time_s = [round(num,0) for num in time_s]
        # create iterable function for determining number of missed timesteps in data
        self.num_miss_timestamp = [(time_s[i+1]-time_s[i])/25.0 for i in
                range(len(time_s) - 1)]
        # or
        #self.num_miss_timestamp = [(t - s)/25.0 for s, t in zip(time_s, time_s[1:])]
        self.newlist = list(map(round, self.num_miss_timestamp))
        print(self.newlist)
#    def bitgen(self, num_miss_timestamp, bits_og):
#        """
#        docstring
#        """
#        if(bits_og):
#            np.ones(self.num_miss_timestamp)
#        else:
#            return  np.zeros(self.num_miss_timestamp)

if __name__ == '__main__':
    #FP ="dataIn/csv/RBG_va_tb_only_DIGITALOUT.csv"
    FP = "dataIn/csv/rffModeling_vaExponentialSource_vaTFF_vaDFF_digitalOut.csv"
    dInst = FillMissData(FP)
    dInst.getdata()
    #result=list(map(dInst.bitgen(),dInst.newlist,dInst.y))
# References
# zip iterator: https://realpython.com/python-zip-function/
