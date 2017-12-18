# Top level makefile, the real shit is at src/Makefile

default: all

.DEFAULT:
	cd src && $(MAKE) $@
	cd test && $(MAKE) $@
install:
	cd src && $(MAKE) $@

.PHONY: install

