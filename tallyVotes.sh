#!/bin/bash

# Έλεγχος αν υπάρχει το αρχείο inputFile στον τρέχοντα κατάλογο
inputFile="inputFile.txt"
if [ ! -f "$inputFile" ]; then
  echo "Το αρχείο inputFile δεν υπάρχει στον τρέχοντα κατάλογο."
  exit 1
fi

# Έλεγχος αν έχει τα κατάλληλα δικαιώματα χρήσης το αρχείο inputFile
if [ ! -r "$inputFile" ]; then
  echo "Δεν έχετε τα απαραίτητα δικαιώματα χρήσης για το αρχείο inputFile."
  exit 1
fi

# Αρχικοποίηση του αρχείου tallyResultsFile
tallyResultsFile="$1"
> "$tallyResultsFile"

# Δημιουργία ενός πίνακα για να αποθηκεύσουμε τα ονόματα που έχουν ήδη ψηφίσει
declare -A votedNames

# Διάβασμα του αρχείου inputFile και καταμέτρηση των ψήφων ανά κόμμα
declare -A tallyResults
while read -r line; do
  name=$(echo "$line" | awk '{$NF=""; print $0}' | xargs)  # Αφαιρούμε την τελευταία στήλη (όνομα κόμματος)
  
  # Έλεγχος αν το όνομα υπάρχει ήδη στον πίνακα votedNames
  if [[ ${votedNames[$name]} ]]; then
    continue  # Αν υπάρχει, προχωράμε στην επόμενη γραμμή
  else
    votedNames[$name]=1  # Σημειώνουμε το όνομα ψηφοφόρου ως ψηφισμένο
  fi
  
  party=$(echo "$line" | awk '{print $NF}')
  ((tallyResults[$party]++))
done < "$inputFile"

# Εγγραφή των αποτελεσμάτων στο αρχείο tallyResultsFile
for party in "${!tallyResults[@]}"; do
  echo "$party ${tallyResults[$party]}"
done | sort -k2,2nr > "$tallyResultsFile"


echo "Τα αποτελέσματα της ψηφοφορίας καταμετρήθηκαν και εγγράφηκαν στο αρχείο "$tallyResultsFile"."
