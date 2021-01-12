#!/usr/local/bin/python3
"""
Analysis main
"""
import parse
import stats_anal as sa
import numpy as np


if __name__ == '__main__':
    #FP ="dataIn/csv/rffModeling_vaExponentialSource_vaTFF_vaDFF_digitalOut.csv"
    #FP = "dataIn/csv/RBG_va_tb_only_DIGITALOUT.csv"
    FP = "dataIn/csv/RBG_va_tb_vtrans500m_only_DIGITALOUT.csv"
    dInst = parse.FillMissData(FP)
    dInst.getdata()
    result=list(map(dInst.complete_bit_array,dInst.newlist,dInst.data))
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
    bias=sa.calc_bias(resultcatbool)
    print('Bit bias from random bitstring is: ', bias)
    expected_bias=sa.calc_expected_bias(0.499, 1e-10, 1e-10, 50e6)
    print('Expected bit bias from random bitstring is: ', expected_bias)
