#!/usr/bin/env bash


make clean
make

#Graph 1
   echo 'M' , 'Misprediction Rate GCC' ,'Misprediction Rate JPEG', 'Misprediction Rate PERL' >> ./graph/graph1/graph1.csv
for i in 7 8 9 10 11 12
 do
  ./sim bimodal ${i} ./proj2-traces/gcc_trace.txt > ./graph/graph1/outputgcc_m${i}.txt
  ./sim bimodal ${i} ./proj2-traces/jpeg_trace.txt > ./graph/graph1/outputjpeg_m${i}.txt
  ./sim bimodal ${i} ./proj2-traces/perl_trace.txt > ./graph/graph1/outputperl_m${i}.txt
    variable1=$(grep 'misprediction rate:' ./graph/graph1/outputgcc_m${i}.txt | awk '{print $3}');
    variable2=$(grep 'misprediction rate:' ./graph/graph1/outputjpeg_m${i}.txt | awk '{print $3}');
    variable3=$(grep 'misprediction rate:' ./graph/graph1/outputperl_m${i}.txt | awk '{print $3}');
    variable4=${i};
    echo "$variable4,$variable3, $variable2, $variable1" >> ./graph/graph1/graph1.csv

  echo "Looping ... number $i"
done
