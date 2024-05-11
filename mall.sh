#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <file> <destination_directory>"
    exit 1
fi

file=$1
dest_dir=$2

if [ ! -f "$file" ]; then
    echo "File not found: $file"
    exit 1
fi

if [ ! -d "$dest_dir" ]; then
    echo "Destination directory not found: $dest_dir"
    exit 1
fi

chmod 777 "$file"

lines=$(wc -l < "$file")
words=$(wc -w < "$file")
chars=$(wc -m < "$file")

is_suspect=0


if [ "$lines" -lt 3 ] && [ "$words" -gt 1000 ] && [ "$chars" -gt 2000 ]; then
    
    keywords=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")
    
    for keyword in "${keywords[@]}"; do
	if grep -qi "$keyword" "$file"; then
            is_suspect=1
            break  
	fi
    done
    
    if LC_ALL=C grep -q '[^[:print:]]' "$file"; then
	is_suspect=1
    fi
fi

chmod 000 "$file"

if [ "$is_suspect" -eq 1 ]; then
    mv "$file" "$dest_dir"
    echo "Moved file: $file"
fi

if [ "$is_suspect" -eq 0 ]; then
   echo "SAFE"
fi