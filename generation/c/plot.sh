gnuplot -e "set terminal png; plot 'test.bin' binary format='%uchar' using 0:1
with boxes" | imgcat
