2>&0
CURRPID=""
for ((INDEX=0;${INDEX}<10;INDEX=${INDEX}+1));do
	./puntini.sh 1 1>&2 &
	read ${CURRPID};
	CURRPID=${CURRPID#*.}
	echo $CURRPID ;
done