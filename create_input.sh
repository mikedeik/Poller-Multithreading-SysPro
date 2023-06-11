#!/bin/bash

# Έλεγχος ορισμάτων εισόδου
if [ "$#" -ne 2 ]; then
  echo "Παρακαλώ δώστε τα σωστά ορίσματα: ./create_input.sh politicalParties.txt numLines"
  exit 1
fi

# Έλεγχος αν το αρχείο politicalParties.txt υπάρχει
if [ ! -f "$1" ]; then
  echo "Το αρχείο $1 δεν υπάρχει."
  exit 1
fi

# Έλεγχος αν το δεύτερο όρισμα είναι έγκυρος ακέραιος αριθμός
re='^[0-9]+$'
if ! [[ $2 =~ $re ]]; then
  echo "Το numLines πρέπει να είναι ένας θετικός ακέραιος αριθμός."
  exit 1
fi

# Διάβασμα του αρχείου politicalParties.txt
readarray -t parties < "$1"

# Αρχικοποίηση του αρχείου εισόδου
inputFile="inputFile.txt"
> "$inputFile"

# Δημιουργία των γραμμών στο αρχείο εισόδου
for ((i=1; i<=$2; i++)); do
  randomName=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w $(( RANDOM % 10 + 3 )) | head -n 1)
  randomParty=${parties[$(( RANDOM % ${#parties[@]} ))]}
  echo "$randomName $randomParty" >> "$inputFile"
done

echo "Το αρχείο εισόδου δημιουργήθηκε με επιτυχία."
