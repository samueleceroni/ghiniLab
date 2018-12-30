while true; do
	./produce.sh &
	wait $!
done