#!/bin/bash
make -f Makefile.cfglp
cd test_files
for i in $(ls *.c)
do
	cd ..
	make -f Makefile.cfg FILE=$i > /dev/null
	./cfglp -d -eval test_files/${i}s306.cfg > our.tok
	./cfglp32 -d -eval test_files/${i}s306.cfg > sir.tok
	diff sir.tok our.tok
	echo $i
	cd test_files
done
cd ..
rm sir.tok 
rm our.tok
exit