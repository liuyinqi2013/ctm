# Top level makefile, the real shit is at src/Makefile

default: all

.DEFAULT:
	make -C src $@
	make -C test $@
	make -C client $@

