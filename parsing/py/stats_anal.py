#!/usr/local/bin/python3
"""
high level support for doing this and that.
"""
# Pouyan Keshavarzian
# EPFL STI IMT AQUA Dec 2020

import numpy as np

def calc_bias(array):
    """
    This function calculates the bit bias of an input np array
    """
    return (abs(len(array)/2.0-np.sum(array)))/len(array)

def calc_expected_bias(vt, tr, tf, fdet):
    """
    This function calculates the expected bias from a random ff
    with an exponential source or rather a uniformally distributed
    tff signal based on vt (sampling threshold), rise and fall times
    of the tff signal (tr tf) and the detection rate i.e. photon
    count rate
    """
    return abs(0.5-vt)*(tr+tf)*fdet/2
