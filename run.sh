#!/bin/bash

# Check if a file argument was provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <FILE>"
    exit 1
fi

FILE=$1

# Check if the file has a .sd extension
if [[ "$FILE" == *.sd ]]; then
    # Execute javaa with the .sd file
    ./parser "$FILE"
    ./javaa "${FILE%.sd}.j" > Assembling.log 2>&1
    
    # Remove .sd extension and execute java
    BASE_FILE="${FILE%.sd}"
    java "$BASE_FILE"
else
    echo "Error: File must have a .sd extension."
fi
