sinclude makefile.config

all: _openidmask _test _openfx

_openidmask:
	make -C openidmask

_openfx:
	make -C openfx

_test: _openidmask
	make -C test

clean:
	make -C openidmask clean
	make -C test clean

install:
	make -C openidmask install
	make -C openfx install

dotest: _test _openidmask
	test/$(VERSION)/test
