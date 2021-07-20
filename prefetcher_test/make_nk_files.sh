#!/bin/bash

mv ${1}kb/testfile_${1}k_0.txt ./temp.txt
rm ${1}kb/*
mv ./temp.txt ${1}kb/testfile_${1}k_0.txt

for ((i=1; i<${2}; i++)); do
	cp ${1}kb/testfile_${1}k_0.txt ${1}kb/testfile_${1}k_${i}.txt
done
