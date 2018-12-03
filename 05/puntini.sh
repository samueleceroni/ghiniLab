#!/bin/bash
SEC=$1
while (( ${SEC} >= 0 )); do
	echo -n ".${BASHPID}"
	sleep 1
	((SEC=${SEC}-1))
done