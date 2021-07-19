#!/bin/bash

mv 4kbs/testfile_4k_0.txt ./temp.txt
rm 4kbs/*
mv ./temp.txt 4kbs/testfile_4k_0.txt

for ((i=1; i<${1}; i++)); do
	cp 4kbs/testfile_4k_0.txt 4kbs/testfile_4k_${i}.txt
done
