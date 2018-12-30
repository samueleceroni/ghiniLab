#!/bin/bash
if (($# == 0)); then echo "devi passare un argomento"; exit 1; fi
./random09.sh >> $1
