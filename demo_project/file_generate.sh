#!/bin/bash
# Simple script for generating a test file
# of input vector coordinate pairs in text format

# Name of the produced output file
OUTFILE="coords.txt"

set -eu

# Function to generate a file with random float x y
# coordinates. Parameters used:
# - min - lower bound for the pseudo-random generator
# - max - upper bound for the pseudo-random generator
# - lim - number of lines (pairs) to be generated
# - dstmp - variable to obtain the start datestamp in ms
# - sn - number of separating spaces to be used in each pair

awk -v min=1.0 -v max=50.0 -v lim=10000 -v dstmp="$(date +%N)" -v sn=1 '
function rand_float (min, max, factor) {
    srand(factor);
    res = (min + rand() * (max - min + 1));
    printf "%05.2f", res
}
BEGIN {
    for (i = 0; i < lim; i++) {
        for (j = 0; j < 2; j++) {
            dstmp += i + j;
            rand_float(min, max, dstmp);
            printf "%*s", j == 0 ? sn : 0, "";
        }
        printf "\n"; }
}' > "${OUTFILE}"
