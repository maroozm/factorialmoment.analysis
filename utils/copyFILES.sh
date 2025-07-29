#!/bin/bash

base_url="alien:///alice/cern.ch/user/s/salman/MC.2010.NEW_BINS_.20250618/output/"
output_base_dir="AnalysisResults"
output_extension=".root"

# List all top-level directories (e.g., 000, 001, ...)
top_dirs=$(alien_ls "$base_url")

for top_dir in $top_dirs; do
  # Create a local output directory for each top-level directory
  local_output_dir="${output_base_dir}_${top_dir}"
  mkdir -p "$local_output_dir"

  # List all subdirectories inside the current top-level directory
  sub_dirs=$(alien_ls "${base_url}${top_dir}/")

  idx=0
  for sub_dir in $sub_dirs; do
    source_url="${base_url}${top_dir}/${sub_dir}/AnalysisResults.root"
    destination_file="${local_output_dir}/AnalysisResults_${idx}${output_extension}"

    # Perform the copy command
    alien_cp "$source_url" "file://$destination_file"
  echo "Copied $source_url to $destination_file"
    idx=$((idx+1))
  done
done