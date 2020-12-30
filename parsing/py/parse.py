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

    def complete_bit_array(self, num_miss_timestamp, original_bits):
        """
        This function is mapped with the list of timesteps to create a complete
        array of random values
        """
        if original_bits:
            return  np.ones(num_miss_timestamp)
        else:
            return  np.zeros(num_miss_timestamp)
