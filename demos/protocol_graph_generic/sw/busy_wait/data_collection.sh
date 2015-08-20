#!/bin/sh

echo "-o 100 -s 100000 -p"
./busy_wait -o 100 -s 100000 -p > samples_o100_s100000.txt
echo "-o 1000 -s 10000 -p"
./busy_wait -o 1000 -s 10000 -p > samples_o1000_s10000.txt
echo "-o 10000 -s 1000 -p"
./busy_wait -o 10000 -s 1000 -p > samples_o10000_s1000.txt
echo "-o 100000 -s 1000 -p"
./busy_wait -o 100000 -s 1000 -p > samples_o100000_s1000.txt
echo "-o 1000000 -s 100 -p"
./busy_wait -o 1000000 -s 100 -p > samples_o1000000_s100.txt


echo "-o 100 -s 100000 -a"
./busy_wait -o 100 -s 100000 -a > average.txt
echo "-o 1000 -s 10000 -a"
./busy_wait -o 1000 -s 10000 -a >> average.txt
echo "-o 10000 -s 1000 -a"
./busy_wait -o 10000 -s 1000 -a >> average.txt
echo "-o 100000 -s 1000 -a"
./busy_wait -o 100000 -s 1000 -a >> average.txt
echo "-o 1000000 -s 100 -a"
./busy_wait -o 1000000 -s 100 -a >> average.txt
