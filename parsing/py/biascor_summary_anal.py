import numpy as np
direc='/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/dataIn/txt/25092021_I0.004Vq0.2Vcas2.1Vh0.2Vr0.5Vt1.1summary/'
biasfile = direc+'bias.txt'
corrfile = direc+'corr.txt'
biasidatanp = np.loadtxt(biasfile).astype(float)
biasdatalist=biasidatanp.tolist()
biasacceptablevals=[x for x in biasdatalist if(abs(x)<=0.001)]
print("number of acceptable bias values is %f" % len(biasacceptablevals))

corrdatanp = np.loadtxt(corrfile).astype(float)
corrdatalist=corrdatanp.tolist()
corracceptablevals=[x for x in corrdatalist if(abs(x)<=0.001)]
print("number of acceptable corr values is %f" % len(corracceptablevals))
indexforacceptablecorr = [i for i in range(len(corrdatalist))
        if(abs(corrdatalist[i])<=0.001)]
# or could do 
# [index for index,value in enumerate(a) if value > 2]
indexforacceptablebias = [i for i in range(len(biasdatalist))
        if(abs(biasdatalist[i])<=0.001)]
# note below solution doesn't work if they are in different postions
# overlappingind= [i for i, j in zip(indexforacceptablebias,indexforacceptablecorr) if i == j]
overlappingind=set(indexforacceptablecorr) & set(indexforacceptablebias)
goodSPADindex = [i+1000 if i < 1800 else (560+i-1800) for i in
        overlappingind]
#print("Good SPADs are:")
#print(sorted(goodSPADindex))
print("number of good RFFs is: %f" % len(goodSPADindex))
goodSPADindex=sorted(goodSPADindex)
datadirecname  = '25092021_I0.004Vq0.2Vcas2.1Vh0.2Vr0.5Vt1.1'
indirec='/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/dataIn/bin/'+datadirecname+'/'
basename = 'Countrate48931157_Sample25MHz_10Mc_BG5MHz_PIXEL_ID_'
goodfiles = [indirec+basename+repr(int(indexval)) for indexval in goodSPADindex]
#print(goodfiles[1])
listoffilestoarrange=" ".join(goodfiles)
outfile='/Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/randAnalysis/dataIn/bin/combined/combined'+datadirecname+'.bin'
commandtorun = "cat "+listoffilestoarrange +" > "+outfile
#print(commandtorun)
#with open("combinegoodrffscript", 'w') as scriptfile:
#    scriptfile.write(commandtorun)
#    scriptfile.close()

# reorder all indices so can plot bias map properly 
indtot= [i for i in range(len(biasdatalist))]
indrearrage = [i+1800 if i < 440 else (i-440) for i in indtot]
biasrearrange=biasdatalist
biasrearrange = [biasrearrange[i] for i in indrearrage]
corrrearrange=corrdatalist
corrrearrange=[corrdatalist[i] for i in indrearrage]
print(max(biasrearrange))
print(min(biasrearrange))
print(max(corrrearrange))
print(min(corrrearrange))
#with open(direc+"biasreordered.txt", 'w') as wfile:
#    for row in biasrearrange:
#        wfile.write(str(row)+'\n')
#    wfile.close()
#
#with open(direc+"corrreordered.txt", 'w') as wfile:
#    for row in corrrearrange:
#        wfile.write(str(row)+'\n')
#    wfile.close()

