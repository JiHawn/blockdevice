#!/bin/bash

for ((i=1; i<${1}; i++)); do
	cp 4kbs/testfile_4k_0.txt 4kbs/testfile_4k_${i}.txt
done
