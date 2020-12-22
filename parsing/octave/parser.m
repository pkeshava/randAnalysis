#!/usr/local/bin/octave
x=csvread("dataIn/csv/bitstream.csv");

randvect= [];
randbitcounter=1;
if(size(argv()) ==0 || argv{1,1} != 'c')
  for i=2:size(x,1)
    if(x(i,3)==1.0 && x(i-1,3)<0.9)
      randvect(randbitcounter)=x(i+3, 2);
      randbitcounter++;
    endif
  end
  fID=fopen("dataOut/parseCadenceSim/bitstream.bin","w");
  fwrite(fID, randvect);
  %random_seg=[];
  %random_seg = reshape(randvect, size(randvect,2)/8, 8)
%else
%  system("cp -R  /Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/cdev/test.bin   /Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/vcdparser/outputs");
%  fID=fopen("dataOut/cETgenerator/cetg.bin","r");
%  randvect=fread(fID);
%  randAES_key=[];
%  %test.bin read as uchar so element array will be 256 bits
%  randAES_key = reshape(randvect, size(randvect,1)/32, 32);
%  [u,i,j] = unique(randAES_key, 'rows', 'first');
%  hasDuplicates = size(u,1) < size(randAES_key,1);
%  iDupRows = setdiff(1:size(randAES_key,1), i);
%  dupRowValues = randAES_key(iDupRows,:)
endif

fclose(fID);


