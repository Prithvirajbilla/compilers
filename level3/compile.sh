#!/bin/bash
make -f Makefile.cfglp
cd test_files
for i in $(ls *.c)
do
	cd ..
	echo $i
	make -f Makefile.cfg FILE=$i > /dev/null
	./cfglp64 -d -ast  test_files/${i}s306.cfg > sir.tok
	# diff -b -B our.tok sir.tok
	cd test_files
done
cd ..
exit
