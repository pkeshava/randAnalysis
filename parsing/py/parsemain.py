#!/usr/local/opt/python@3.9/bin/python3
"""
Analysis main
"""
import glob
import parse
import stats_anal as sa
import numpy as np


if __name__ == '__main__':
    #FP = "dataIn/csv/thresholdAnalysis/RBG_va_tb_vtrans425m_only_DIGITALOUT.csv"
    #filepaths= sorted(list(glob.iglob('dataIn/csv/thresholdAnalysis/*.csv')))
    filepaths= sorted(list(glob.iglob('dataIn/csv/thresholdAnalysis_trtf200ns/*.csv')))
    vt= np.array([0.200, 0.225, 0.250, 0.275, 0.300, 0.325, 0.350, 0.375, 0.400,
        0.425, 0.450, 0.475, 0.499])
    vts=np.array(["vt200m","vt225m","vt250m","vt275m","vt300m","vt325m","vt350m",
        "vt375m","vt400m","vt425m","vt450m","vt475m","vt499m"])
    eb = np.empty(shape=[len(vt), 1])
    sb = np.empty(shape=[len(vt), 1])
    ac1 = np.empty(shape=[len(vt), 1])
    #ac2 = np.empty(shape=[100000,1])
    lags=range(2)

    for i in range(len(vt)):
        FP = filepaths[i]
        dInst = parse.FillMissData(FP)
        dInst.getdata()
        result = list(map(dInst.complete_bit_array, dInst.newlist, dInst.data))
        resultcat = np.concatenate(result, axis=0)
        resultround = resultcat.round()
        resultcatbool = np.array(resultround, dtype=np.ubyte)
        # print('Concatenated bits: ', resultcatbool)
        resultcatpacked = np.packbits(resultcatbool)
        print(resultcatpacked)
        # FPW="dataOut/parseCadenceSim/RBG_verilogAMS_expSource{}.bin".format(vts[i])
        FPW = "dataOut/parseCadenceSim/RBG_verilogAMS_expSource{}.bin".format(vts[i])
        resultcatpacked.tofile(FPW)
        resultimport = np.fromfile(FPW, dtype=np.ubyte)
        print('Array imported:', resultimport)
        bias = sa.calc_bias(resultcatbool)
        sb[i] = bias
        print('Bit bias from random bitstring is: ', bias)
        vtvalue = vt[i]
        expected_bias = sa.calc_expected_bias(vtvalue, 2e-10, 2e-10, 50e6)
        eb[i] = expected_bias
        print('Expected bit bias from random bitstring is: ', expected_bias)
        autocorr = sa.autocorr1(resultcatbool, lags)
        print('1d bit lag autocorrelation from random bitstring is: ', autocorr)
print(eb)
print(sb)
#np.savetxt('dataIn/txt/expectedbias_trtf200ns.txt', eb, delimiter=',')
#np.savetxt('dataIn/txt/simulatedbias_trtf200ns.txt', sb, delimiter=',')
