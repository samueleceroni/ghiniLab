if [[ -e out.txt ]]; then rm out.txt; fi;

for i in {1..10}; do ./genera.sh out.txt; done
