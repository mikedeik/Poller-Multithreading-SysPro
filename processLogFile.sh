#!/bin/bash

# Έλεγχος αν υπάρχει το αρχείο poll-log
if [ ! -f "$1" ]; then
    echo "Το αρχείο poll-log δεν υπάρχει"
    exit 1
fi

# Έλεγχος δικαιωμάτων πρόσβασης στο αρχείο poll-log
if [ ! -r "$1" ]; then
    echo "Δεν έχετε τα κατάλληλα δικαιώματα πρόσβασης στο αρχείο poll-log"
    exit 1
fi

# Όνομα του αρχείου που θα περιέχει τα αποτελέσματα
pollerResultsFile="pollerResultsFile.txt"

# Διαγραφή του αρχείου αποτελεσμάτων αν υπάρχει
if [ -f "$pollerResultsFile" ]; then
    rm "$pollerResultsFile"
fi

# Δημιουργία ενός πίνακα για να αποθηκεύσουμε τα ονόματα που έχουν ήδη ψηφίσει
declare -A votedNames

declare -A pollerResults
# Διάβασμα του αρχείου poll-log και επεξεργασία των δεδομένων
while read -r line; do

    name=$(echo "$line" | awk '{$NF=""; print $0}' | xargs)  # Αφαιρούμε την τελευταία στήλη (όνομα κόμματος)

    # Έλεγχος αν το όνομα υπάρχει ήδη στον πίνακα votedNames
    if [[ ${votedNames[$name]} ]]; then
      continue  # Αν υπάρχει, προχωράμε στην επόμενη γραμμή
    else
      votedNames[$name]=1  # Σημειώνουμε το όνομα ψηφοφόρου ως ψηφισμένο
    fi

    party=$(echo "$line" | awk '{print $NF}')
    ((pollerResults[$party]++))
    
done < "$1"

# Εγγραφή των αποτελεσμάτων στο αρχείο pollerResultsFile
for party in "${!pollerResults[@]}"; do
  echo "$party ${pollerResults[$party]}"
done | sort -k2,2nr > "$pollerResultsFile"

echo "Ολοκλήρωση επεξεργασίας του αρχείου poll-log"
