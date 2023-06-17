#!/bin/bash

# check for the args
if [ "$#" -ne 2 ]; then
  echo "Usage: ./create_input.sh politicalParties.txt numLines"
  exit 1
fi

# check if the file politicalParties.txt exists
if [ ! -f "$1" ]; then
  echo "file $1 does not exist."
  exit 1
fi

# check if second argument is a number
re='^[0-9]+$'
if ! [[ $2 =~ $re ]]; then
  echo "second argument must be a number"
  exit 1
fi

# Read file politicalParties.txt
readarray -t parties < "$1"

# Setting up the output file
inputFile="inputFile.txt"
> "$inputFile"

# Create the lines of the output file
for ((i=1; i<=$2; i++)); do
  firstName=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w $(( RANDOM % 10 + 3 )) | head -n 1)
  lastName=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w $(( RANDOM % 10 + 3 )) | head -n 1)
  randomParty=${parties[$(( RANDOM % ${#parties[@]} ))]}
  echo "$firstName $lastName $randomParty" >> "$inputFile"
done

echo "File created."
