#!/bin/bash
for ((i=1; i<33; i++)); do
	sudo hdparm --fibmap testfile_4k_$i
done
