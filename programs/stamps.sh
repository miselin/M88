#!/bin/bash

SHA=$(git rev-parse --short HEAD)

# generate build stamp file
cat <<EOF >build/stamp.asm
bits 16
cpu 8086

section .description

db "Matt BIOS version ${SHA}"

section .romdate

db "$(date "+%m/%d/%y")"

EOF
