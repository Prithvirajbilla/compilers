#!/bin/bash
cd level-0-32bits
make -f Makefile.cfglp
cd test_files
for i in $(ls *.c)
do
	cd ..
	make -f Makefile.cfg FILE=$i > /dev/null
	./cfglp -d -tokens test_files/${i}s306.cfg > our.tok
	./cfglp32 -d -tokens test_files/${i}s306.cfg > sir.tok
	diff sir.tok our.tok
	echo $i
	cd test_files
done

exit