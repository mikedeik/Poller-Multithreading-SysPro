#!/bin/bash

# Check if we have correct num of args
if [ "$#" -ne 1 ]; then
  echo "Usage: ./tallyVotes.sh [tallyResultsFile]"
  exit 1
fi

# Check if the input file exists
inputFile="inputFile.txt"
if [ ! -f "$inputFile" ]; then
  echo "file $inputFile does not exist in this directory"
  exit 1
fi

# check if we have rights for reading the file
if [ ! -r "$inputFile" ]; then
  echo "you have no rights for reading this file."
  exit 1
fi

# Set up the tally results file
tallyResultsFile="$1"
> "$tallyResultsFile"

# create an array to store the names that have voted
declare -A votedNames

# and another one for the results and a var for total
declare -A tallyResults

total=0;

while read -r line; do
  name=$(echo "$line" | awk '{$NF=""; print $0}' | xargs)  # get the name of the voter
  
  # check if the voter allready exists and continue to next if yes
  if [[ ${votedNames[$name]} ]]; then
    continue  
  else
    votedNames[$name]=1  # Tag the voter that he voted
  fi
  
  party=$(echo "$line" | awk '{print $NF}')
  ((tallyResults[$party]++))
  ((total++))
done < "$inputFile"

# Output the results
for party in "${!tallyResults[@]}"; do
  echo "$party ${tallyResults[$party]}"
done | sort -k2,2nr > "$tallyResultsFile"

echo "Total : $total" >> "$tallyResultsFile"
echo "Done calculating results to "$tallyResultsFile"."
