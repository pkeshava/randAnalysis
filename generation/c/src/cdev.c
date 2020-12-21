/* © Pouyan Keshavarzian 2019
PhD Student EPFL

This code is used as a first step dive into the world of true random number generation
*/

#include "config.h"

int main(int argc, char** argv){

int nBytes;
float prob1s;

stateVariables stateVars = {.nBytes = 0, .argc = argc, .prob1s = 0.5, .argv = argv};

stateVars = determineSequence(stateVars);
nBytes = stateVars.nBytes;
prob1s = stateVars.prob1s;

// create a seed by reading epoch time
srand((unsigned int) time (NULL));
// Generate random bits
// also prints bytes as ascii characters
u8* pu8Bstream;
printf("\n");
//pu8Bstream = pu8GenRdmAsciiByteStream(nBytes);
pu8Bstream = pu8GenBiasedRdmBitStream(nBytes, stateVars.prob1s); // get biasedStream
printf("\n");
// prints a series of bytes as the decimal representation of the ascii byte i.e 122 104 120 etc..
if(nBytes >= 100){
  printf("\nNumber of bytes too long to print will only save to file");}
else{
for(int i = 0; i<nBytes; i++)
  printf("%u ", pu8Bstream[i]);}
printf("\n\n");
// write to a binary file and then read back to check it was done correctly
writeToFile(pu8Bstream, nBytes);
u8* buffer;
buffer = readFromFile(nBytes);
// This now should print exactly the same decimal representations of ascii bytes as done before.
if(nBytes >= 100){
  printf("\nNumber of bytes too long to print will only save to file");}
else{
for(int i = 0; i<nBytes; i++)
  printf("%u ", buffer[i]);
printf("\n\n");
// Now convert the bytes to bit representation and print on the command line
printBits(nBytes, buffer); // NOTE if this is done without the dynamic memory allocation then you need to have &buffer
}
printf("\n");

free(pu8Bstream);
free(buffer);

return 0;

}
