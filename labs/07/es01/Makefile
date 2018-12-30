all: tutto.txt

tutto.txt: B1.txt B2.txt
	touch tutto.txt
	cat B1.txt > tutto.txt
	cat B2.txt >> tutto.txt

B1.txt: A1.txt
	cat A1.txt > B1.txt
	date >> B1.txt

B2.txt: A2.txt
	cat A2.txt > B2.txt
	date >> B2.txt

A1.txt:
	touch A1.txt
	echo AAAA > A1.txt

A2.txt:
	touch A2.txt
	echo ZZZZ > A2.txt

.PHONY: clean

clean:
	-rm A1.txt A2.txt

touch:
	touch -c B1.txt
