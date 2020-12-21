randvect= [];
randbitcounter=1;

system("cp -R  /Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/cdev/test.bin   /Users/Pouyan/Builds/PhD/research_PhD/builds_PhD/vcdparser/outputs");
fID=fopen("outputs/test.bin","r");
randvect=fread(fID);
randomAES_key=[];
%test.bin read as uchar so element array will be 256 bits
randAES_key = reshape(randvect, size(randvect,1)/32, 32);
[u,i,j] = unique(randAES_key, 'rows', 'first');
hasDuplicates = size(u,1) < size(randAES_key,1);
iDupRows = setdiff(1:size(randAES_key,1), i);
dupRowValues = randAES_key(iDupRows,:)


fclose(fID);
