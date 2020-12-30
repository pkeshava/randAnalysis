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
