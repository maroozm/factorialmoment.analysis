#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default path
path="./"

# Parse options
while getopts "p:" opt; do
  case ${opt} in
    p )
      path="${OPTARG}"
      ;;
    \? )
      echo "Invalid option: $OPTARG" 1>&2
      ;;
  esac
 done

# Ask for number of bins
read -p "Enter the number of bins [default: 1]: " num_bins
if [[ -z "$num_bins" ]]; then
  num_bins=1
fi
bins=()
for ((i=1; i<=num_bins; i++)); do
  bins+=("bin${i}")
done

echo -e "${YELLOW}Scanning directories in ${path}...${NC}"

selected_dirs=()

# Find candidate directories
for dir in "$path"*/; do
  dir=${dir%/} # Remove trailing slash
  found=0
  # Check subdirectories for Fqe_standardErrorSample*.root
  for sub in "$dir"/*/datafiles/; do
    sub=${sub%/}
    if compgen -G "$sub/Fqe_standardErrorSample*.root" > /dev/null; then
      found=1
      break
    fi
  done
  if [[ $found -eq 1 ]]; then
    read -p "Include directory $dir? (Y/n): " yn
    yn=${yn:-Y}
    case $yn in
      [Yy]*) selected_dirs+=("$dir"); ;;
      *) ;;
    esac
  fi
done

for dir in "${selected_dirs[@]}"; do
  for bin in "${bins[@]}"; do
    echo -e "${BLUE}Processing directory: $dir${NC}"
    echo -e "${YELLOW}Merging files...${NC}"
    hadd "$dir/$bin/datafiles/Fqe_standardErrorSample.root" "$dir/$bin/datafiles/Fqe_standardErrorSample"*.root
    echo -e "${GREEN}Created $dir/$bin/datafiles/Fqe_standardErrorSample.root successfully.${NC}"
  done
done