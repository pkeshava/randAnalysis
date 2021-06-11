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
import subprocess

#%%
    
def calc_bias(array):
    """
    This function calculates the bit bias of an input np array
    More specifically the probabiliy of ones
    """
    x = (len(array)/2.0-np.sum(array))/len(array)
    return 0.5-x

def calc_expected_bias(vt, tr, tf, fdet):
    """
    This function calculates the expected bias from a random ff
    with an exponential source or rather a uniformally distributed
    tff signal based on vt (sampling threshold), rise and fall times
    of the tff signal (tr tf) and the detection rate i.e. photon
    count rate
    """
    bias = (0.5-vt)*(tr+tf)*fdet/2
    prob_of_ones = 0.5+(0.5-vt)*(tr+tf)*fdet/2
    return bias, prob_of_ones
    

def bitfield(n):
    return [1 if digit=='1' else 0 for digit in bin(n)[2:]]
#%%
# tr, tf, eta
rff_mu = [4e-11, 4e-11, 0.505]
# tr, tf, eta
rff_sigma = [5e-12, 5e-12, 0.025]
num_of_pixels = 32
percent_hot = 0.01
percent_screaming = 0.001
sampling_rates = [10e6, 20e6, 30e6, 40e6, 50e6]
count_rates = [100e5, 1e6, 10e6, 50e6, 80e6]

#num_bytes_to_generate = np.array([1310720])
num_bytes_to_generate = np.array([102400])
num_bits = 8*num_bytes_to_generate[0]

#num_bits_generated = np.array([1024])
# djenrandom generates in 2kB blocks i.e 500 = 1MB
#num_byte_blocks = np.array([1])


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

# For a number of BIT_GEN_CYCLES           

#%% Calculate expected bias per pixel for 

def calc_exp_bias_perPix(pixel_count, rff_mu, rff_sigma, count_rate):
    exp_biases = np.array([])
    probs_of_ones = np.array([])
    for i, pixel_count in enumerate(pixel_count):
        tr = np.random.normal(rff_mu[0], rff_sigma[0])
        tf = np.random.normal(rff_mu[1], rff_sigma[1])
        vt = np.random.normal(rff_mu[2], rff_sigma[2])
        fdet = count_rate
        exp_bias, prob_of_ones = calc_expected_bias(vt, tr, tf, fdet)
        exp_biases = np.append(exp_biases,[exp_bias])
        probs_of_ones = np.append(probs_of_ones,[prob_of_ones])
    return exp_biases, probs_of_ones

exp_biases, probs_of_ones = calc_exp_bias_perPix(pixel_count, rff_mu, rff_sigma, count_rates[4])


#%%
def convert_values_to_str(probs_of_ones, num_bits_generated):
    prob_ones_str= ["".join(item) for item in probs_of_ones.astype(str)]
    num_bits_str= ["".join(item) for item in num_bits_generated.astype(str)]
    return prob_ones_str, num_bits_str
# More general form:
def convert_array_vals_to_str(arry):
    arry_str= ["".join(item) for item in arry.astype(str)]
    return arry_str

def gen_bits_with_bias(num_bytes_to_generate,prob_of_ones, pixel_num):
    if(num_bytes_to_generate[0]>100000000):
        print("TOO MANY BITS!")
        return
    elif(num_bytes_to_generate[0]>131072 and num_bytes_to_generate[0]<=100000000):
        num_calls = num_bytes_to_generate[0] // 131072
        remainder = num_bytes_to_generate[0] % 131072
        bits_concat = np.array([], dtype=bool)
        for i in range(num_calls):
            prob_ones_str, num_bits_str = convert_values_to_str(prob_of_ones, num_bytes_to_generate)
            #num_bits_str = convert_array_vals_to_str(num_bits_to_generate)
            #prob_ones_str = convert_array_vals_to_str(prob_of_ones)
            args = ("/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/generation/c/RNG", "-N", '131072', "-p", prob_ones_str[0])
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            popen.wait()
            xbash = np.fromfile('dataOut/cETgenerator/cetg.bin', dtype='uint8')
            p = np.unpackbits(xbash, axis=0)
            bits_concat= np.append(bits_concat,p)
        if(remainder > 0):
            prob_ones_str, num_bits_str = convert_values_to_str(probs_of_ones, remainder)
            args = ("/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/generation/c/RNG", "-N", num_bits_str[0], "-p", prob_ones_str[0])
            popen = subprocess.Popen(args, stdout=subprocess.PIPE)
            popen.wait()
            xbash = np.fromfile('dataOut/cETgenerator/cetg.bin', dtype='uint8')
            p = np.unpackbits(xbash, axis=0)
            bits_concat= np.append(bits_concat,p)
    
        bias_from_file = calc_bias(bits_concat)
    
    else:
        prob_ones_str, num_bits_str = convert_values_to_str(prob_of_ones, num_bytes_to_generate)
        args = ("/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/generation/c/RNG", "-N", num_bits_str[0], "-p", prob_ones_str[0])
        popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        popen.wait()
        xbash = np.fromfile('dataOut/cETgenerator/cetg.bin', dtype='uint8')
        #xbash = np.fromfile('dataOut/djen/djen.bin', dtype='uint8')
        p = np.unpackbits(xbash, axis=0)
        bits_concat= p
        bias_from_file = calc_bias(p)
    
    num_bits_generated = num_bytes_to_generate[0]*8
    
    filename = '/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/modelling/dataOut/cETgenerator/SPADdata/spad{}.bin'.format(pixel_num)
    filenametxt = '/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/modelling/dataOut/cETgenerator/SPADdata/spad{}.txt'.format(pixel_num)

    bits_concat.astype('uint8').tofile(filename)
    np.savetxt(filenametxt, bits_concat, delimiter=',', fmt='%1.0u')
    
    
    print("Requested bit bias:")
    print(probs_of_ones[0])
    print("Generated bit bias:")
    print(bias_from_file)
    delta = bias_from_file-prob_of_ones[0]
    return delta, num_bits_generated, bias_from_file
    
#%%
import time
from random import SystemRandom
cryptogen = SystemRandom()
for pixel_num in range(num_of_pixels):
    delta, num_bits_from_file, bias_from_file = gen_bits_with_bias(np.array([num_bytes_to_generate[0]]),np.array([probs_of_ones[0]]), pixel_num)
    #timeforsleep = random.uniform(1.4757698908065398081,0.902123422) # Sleep for 3 seconds
    timeforsleep = 352.9136786423423123056456495*cryptogen.random()
    time.sleep(timeforsleep) # Sleep for 3 seconds


#%%
def gen_keys(num_of_pixels,num_bits):
    bitBeingAnal = np.zeros(num_of_pixels, dtype='uint8')
    key = np.zeros(num_bits, dtype='uint32')
    
    for bit in range(num_bits):
        for pixel in range(num_of_pixels):
            xbash = np.fromfile('/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/modelling/dataOut/cETgenerator/SPADdata/spad{}.bin'.format(pixel), dtype='uint8')
            bitBeingAnal[pixel] = xbash[bit]
        # the -1 is because of the machine endian... otherwise when you imply .view it doesn't work...
        shaped_into_bytes =  np.packbits(bitBeingAnal.reshape(1, 4, 8)[:, ::-1])
        key[bit] = shaped_into_bytes.view(np.uint32)
        #key[bit] = np.packbits(bitBeingAnal, axis=0,dtype=np.uint8)
    return key

keygenfuncvalues=gen_keys(num_of_pixels,num_bits)
unique_keys_generated, counts_for_each_key = np.unique(keygenfuncvalues, return_counts=True)
most_common_key =unique_keys_generated[counts_for_each_key.argmax()]

def estimate_min_entropy(num_of_pixels, counts_for_each_key):
    number_of_possible_keys = 2**num_of_pixels
    p = max(counts_for_each_key)/number_of_possible_keys
    min_entropy_simulated_based_on_biased_bits = -np.log2(p)
    return min_entropy_simulated_based_on_biased_bits
#%%

min_entropy_simulated_based_on_biased_bits = estimate_min_entropy(num_of_pixels, counts_for_each_key)

