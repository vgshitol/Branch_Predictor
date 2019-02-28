#!/usr/bin/env bash

mkdir graph graph/graph1 graph/graph2

make clean
make

#Graph 1
   echo 'M' , 'Misprediction Rate GCC' ,'Misprediction Rate JPEG', 'Misprediction Rate PERL' >> ./graph/graph1/graph1.csv
for i in 7 8 9 10 11 12 13 14 15 16
 do
  ./sim bimodal ${i} ./proj2-traces/gcc_trace.txt > ./graph/graph1/outputgcc_m${i}.txt
  ./sim bimodal ${i} ./proj2-traces/jpeg_trace.txt > ./graph/graph1/outputjpeg_m${i}.txt
  ./sim bimodal ${i} ./proj2-traces/perl_trace.txt > ./graph/graph1/outputperl_m${i}.txt
    variable1=$(grep 'misprediction rate:' ./graph/graph1/outputgcc_m${i}.txt | awk '{print $3}');
    variable2=$(grep 'misprediction rate:' ./graph/graph1/outputjpeg_m${i}.txt | awk '{print $3}');
    variable3=$(grep 'misprediction rate:' ./graph/graph1/outputperl_m${i}.txt | awk '{print $3}');
    variable4=${i};
    echo "$variable4,$variable1, $variable2, $variable3" >> ./graph/graph1/graph1.csv

  echo "Looping ... number $i"
done

#Graph 2
   echo 'M' , 'N', 'Misprediction Rate GCC' ,'Misprediction Rate JPEG', 'Misprediction Rate PERL' >> ./graph/graph2/graph2.csv
for i in 7 8 9 10 11 12 13 14 15 16
 do
 for j in 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
 do
 if (($i >= $j)); then

  ./sim gshare ${i} ${j} ./proj2-traces/gcc_trace.txt > ./graph/graph2/outputgcc_m${i}${j}.txt
  ./sim gshare ${i} ${j} ./proj2-traces/jpeg_trace.txt > ./graph/graph2/outputjpeg_m${i}${j}.txt
  ./sim gshare ${i} ${j} ./proj2-traces/perl_trace.txt > ./graph/graph2/outputperl_m${i}${j}.txt
    variable1=$(grep 'misprediction rate:' ./graph/graph2/outputgcc_m${i}${j}.txt | awk '{print $3}');
    variable2=$(grep 'misprediction rate:' ./graph/graph2/outputjpeg_m${i}${j}.txt | awk '{print $3}');
    variable3=$(grep 'misprediction rate:' ./graph/graph2/outputperl_m${i}${j}.txt | awk '{print $3}');
    variable4=${j};
    variable5=${i};
    echo "$variable5, $variable4, $variable1, $variable2, $variable2" >> ./graph/graph2/graph2.csv

  echo "Looping ... number $i $j"
fi
 done
done