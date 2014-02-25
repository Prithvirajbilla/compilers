#!/bin/bash
make -f Makefile.cfglp
cd test_files
for i in $(ls *.c)
do
	cd ..
	echo $i
	make -f Makefile.cfg FILE=$i > /dev/null
	./cfglp -d -eval test_files/${i}s306.cfg > our.tok
	./cfglp64 -d -eval test_files/${i}s306.cfg > sir.tok
	diff -b -B our.tok sir.tok
	cd test_files
done
cd ..
rm sir.tok 
rm our.tok
exit
