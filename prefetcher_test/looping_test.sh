#!/bin/bash

for ((i=0; i<${1}; i++)); do
    echo 1 | sudo tee /proc/sys/vm/drop_caches
    ./read_4kb
	echo 1 | sudo tee /proc/sys/vm/drop_caches
	./prefetcher_4kbs & ./read_4kb
done
