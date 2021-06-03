#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jun  2 08:41:44 2021

@author: Pouyan

Modelling TC3 chip
 
"""
import numpy as np
import random
from math import sqrt
# tr, tf, eta
rff_mu = [4e-11, 4e-11, 0.505]
# tr, tf, eta
rff_sigma = [5e-12, 5e-12, 0.025]
num_of_pixels = 256
percent_hot = 0.01
percent_screaming = 0.001
sampling_rates = [10e6, 20e6, 30e6, 40e6, 50e6]
count_rates = [100e5, 1e6, 10e6, 50e6, 80e6]

# determine number of hot pixels and screamers using a binomial distribution
num_hot = np.random.binomial(num_of_pixels, percent_hot)
num_scream = np.random.binomial(num_of_pixels, percent_screaming)
# assign the number of hot pixels and screamers randomly in space
def assign_hotAndScream_pixelLocations(num_hot, num_scream, num_of_pixels):
    pos_hot = random.sample(range(1, 256), num_hot)
    while True:
        notUnique = 0
        pos_scream = random.sample(range(1, num_of_pixels), num_scream)
        for i in range(num_scream):
            for j in range(num_hot):
                if(pos_scream[i] == pos_hot[j]):
                    notUnique = 1    
        if(~notUnique):
            print("Unique")
            return pos_hot, pos_scream
        else:
            print("Generated locations of screamers and hot pixels not unique...trying again")
            
pos_hot, pos_scream = assign_hotAndScream_pixelLocations(num_hot, num_scream, num_of_pixels)

# assign count rates

def assign_count_rates(pos_hot,pos_scream, num_of_pixels, count_rate):
    pixel_count_rates = np.array([])
    shot_noise = sqrt(count_rate)
    for i in range(num_of_pixels):
        s = np.random.normal(count_rate, shot_noise)
        exits_in_hot = i in pos_hot
        exits_in_scream = i in pos_scream
        if(exits_in_hot):
            count = s+np.random.normal(count_rate*10, sqrt(10*count_rate))
            pixel_count_rates = np.append(pixel_count_rates,[count])
        elif(exits_in_scream):
            count = s+np.random.normal(count_rate*100, sqrt(100*count_rate))
            pixel_count_rates = np.append(pixel_count_rates,[count])
        else:
            count = s
            pixel_count_rates= np.append(pixel_count_rates,[count])
    return pixel_count_rates

pixel_count = assign_count_rates(pos_hot,pos_scream, num_of_pixels, count_rates[0])
            
    

#%%

    
def calc_bias(array):
    """
    This function calculates the bit bias of an input np array
    """
    return (len(array)/2.0-np.sum(array))/len(array)

def calc_expected_bias(vt, tr, tf, fdet):
    """
    This function calculates the expected bias from a random ff
    with an exponential source or rather a uniformally distributed
    tff signal based on vt (sampling threshold), rise and fall times
    of the tff signal (tr tf) and the detection rate i.e. photon
    count rate
    """
    return (0.5-vt)*(tr+tf)*fdet/2