#!/bin/bash

# Check if the poll-log file exists
if [ ! -f "$1" ]; then
    echo "poll-log file does not exist"
    exit 1
fi

# Checking if we have read rights for the file
if [ ! -r "$1" ]; then
    echo "No read rights for this file"
    exit 1
fi

# Set up the output file and remove it if it exists
pollerResultsFile="pollerResultsFile.txt"

if [ -f "$pollerResultsFile" ]; then
    rm "$pollerResultsFile"
fi

# Create tables with names that have allready voted and their collected votes
declare -A votedNames

declare -A pollerResults
# Read the lines of the file
while read -r line; do

    # name is whole libe but last column 
    name=$(echo "$line" | awk '{$NF=""; print $0}' | xargs)  

    # Check if the name exists in array votedNames if it does continue
    if [[ ${votedNames[$name]} ]]; then
      continue  
    else
      votedNames[$name]=1  # else mark it as voted 
    fi
    # name of the party is last column of the line
    party=$(echo "$line" | awk '{print $NF}')
    ((pollerResults[$party]++))
    
done < "$1"

# output the results in the pollerResults file
for party in "${!pollerResults[@]}"; do
  echo "$party ${pollerResults[$party]}"
done | sort -k2,2nr > "$pollerResultsFile"

echo "Ολοκλήρωση επεξεργασίας του αρχείου poll-log"
