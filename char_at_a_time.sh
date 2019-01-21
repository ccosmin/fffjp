#!/bin/bash
# data file
INPUT=data/citylots.json

# while loop
while IFS= read -r -n1 char
do
    # display one character at a time
    echo  "$char"
done < "$INPUT"

