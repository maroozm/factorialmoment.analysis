#!/bin/bash

batch_size=100

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

for dir in AnalysisResults_*; do
  if [ -d "$dir" ]; then
    echo -e "${BLUE}Processing directory: $dir${NC}"
    files=("$dir"/*.root)
    total_files=${#files[@]}
    batch=0
    for ((i=0; i<total_files; i+=batch_size)); do
      batch_files=("${files[@]:i:batch_size}")
      output_file="merged_${dir#AnalysisResults_}_${batch}.root"
      echo -e "${YELLOW}Merging files ${i} to $((i+batch_size-1 < total_files ? i+batch_size-1 : total_files-1)) from $dir into $output_file${NC}"
      hadd "$output_file" "${batch_files[@]}"
      if [ $? -eq 0 ]; then
        echo -e "${GREEN}Created $output_file successfully.${NC}"
      else
        echo -e "${RED}Failed to create $output_file.${NC}"
      fi
      batch=$((batch+1))
    done
  fi
done 
