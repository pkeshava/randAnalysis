#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Aug  6 13:59:01 2021

@author: Pouyan
"""

import numpy as np

#%% MATRIX MATH USING PYTHON AND OCTAVE COMPARISON

# matrix multiplication in python is stupid
# https://stackoverflow.com/questions/36384760/transforming-a-row-vector-into-a-column-vector-in-numpy

x = np.array([3,3,3],dtype=float)
y = np.array([1,2,1],dtype=float)
xtrans = x[..., None]
ytrans = y[..., None]

innerproduct=x@ytrans
outerproduct=xtrans * y

import oct2py
from oct2py import octave
oc = oct2py.Oct2Py()
octave.addpath('/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/modelling')
#ip_oct = np.zeros(1, dtype=float)
#op_oct = np.zeros((3, 3), dtype=float)
#octave.run('matrixmath.m')
ip_oct, op_oct = octave.matrixmath(x, y, nout=2)
#%%

biasfile = '/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/dataIn/txt/bias.txt'
corrfile = '/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/dataIn/txt/corr.txt'


bias = np.loadtxt(biasfile).astype(float)
corr = np.loadtxt(corrfile).astype(float)

biasr = np.reshape(bias, (32,70))
x = np.linspace(1,70,70, dtype=float)
#y = np.linspace(1,32,32,dtype=float)

arraytotex = np.empty((70,3))
# y = np.full((70,1), 1)
# #arraytotex[:::] = x[:]:i[:]:biasr[1,:].T
# arraytotex[:,0] = x
# arraytotex[:,1] = y[:,0]
# arraytotex[:,2] = biasr[0,:]

file = '/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/dataOut/txtDataForPlot/biasfortikz.txt'
with open(file, 'w') as f:
    for i in range(32):
        y = np.full((70,1), i+1)
        arraytotex[:,0] = x
        arraytotex[:,1] = y[:,0]
        arraytotex[:,2] = biasr[i,:]
        np.savetxt(f, arraytotex)
        f.write("\n")
    f.close()
        
        
    
    