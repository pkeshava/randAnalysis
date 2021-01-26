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


def autocorrlag(x, t):
    """
    calculates autocorrelation
    """
    return np.corrcoef(np.array([x[:-t], x[t:]]))


def autocorr(x):
    result = np.correlate(x, x, mode='full')
    return result[int(result.size/2):]


def autocorr1(x, lags):
    '''numpy.corrcoef, partial'''

    corr = [1. if lag == 0 else np.corrcoef(x[lag:], x[:-lag])[0][1] for lag in lags]
    return np.array(corr)

def autocorr2(x, lags):
    '''manualy compute, non partial'''

    mean = np.mean(x)
    var = np.var(x)
    xp = x-mean
    corr = [1. if lag == 0 else np.sum(xp[lag:]*xp[:-lag])/len(x)/var for lag in lags]

    return np.array(corr)
