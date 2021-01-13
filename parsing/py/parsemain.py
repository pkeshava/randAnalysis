#!/usr/local/bin/python3
"""
Analysis main
"""
import parse
import stats_anal as sa
import numpy as np
import glob

if __name__ == '__main__':
    #FP = "dataIn/csv/thresholdAnalysis/RBG_va_tb_vtrans425m_only_DIGITALOUT.csv"
    filepaths= sorted(list(glob.iglob('dataIn/csv/thresholdAnalysis/*.csv')))
    vt= np.array([0.200, 0.225, 0.250, 0.275, 0.300, 0.325, 0.350, 0.375, 0.400,
        0.425, 0.450, 0.475, 0.499])
    vts=np.array(["vt200m","vt225m","vt250m","vt275m","vt300m","vt325m","vt350m","vt375m","vt400m","vt425m","vt450m","vt475m","vt499m"])

    for i in range(len(vt)):
        FP=filepaths[i]
        dInst = parse.FillMissData(FP)
        dInst.getdata()
        result=list(map(dInst.complete_bit_array,dInst.newlist,dInst.data))
        resultcat = np.concatenate( result, axis=0)
        resultround = resultcat.round()
        resultcatbool=np.array(resultround, dtype=np.ubyte)
        #print('Concatenated bits: ', resultcatbool)
        resultcatpacked=np.packbits(resultcatbool)
        print(resultcatpacked)
        #FPWa="dataOut/parseCadenceSim/pythonParseCadenceSim.bin"
        FPW="dataOut/parseCadenceSim/RBG_verilogAMS_expSource{}.bin".format(vts[i])
        resultcatpacked.tofile(FPW)
        resultimport = np.fromfile(FPW, dtype=np.ubyte)
        print('Array imported:', resultimport)
        bias=sa.calc_bias(resultcatbool)
        print('Bit bias from random bitstring is: ', bias)
        vtvalue=vt[i]
        expected_bias=sa.calc_expected_bias(vtvalue, 1e-10, 1e-10, 50e6)
        print('Expected bit bias from random bitstring is: ', expected_bias)
