#!/bin/bash
make -f Makefile.cfglp
cd test_files
for i in $(ls *.c)
do
	cd ..
	echo $i
	make -f Makefile.cfg FILE=$i > /dev/null
	./cfglp  -icode test_files/${i}s306.cfg
	#cp test_files/${i}s306.cfg.spim test_files/${i}s306.cfg.spim.our
	cp test_files/${i}s306.cfg.ic test_files/${i}s306.cfg.ic.our
	./cfglp64 -icode test_files/${i}s306.cfg 
	#diff -b -B test_files/${i}s306.cfg.spim.our test_files/${i}s306.cfg.spim
	diff -b -B test_files/${i}s306.cfg.ic.our test_files/${i}s306.cfg.ic
	cd test_files
done
cd ..
exit
