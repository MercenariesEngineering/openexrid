sinclude makefile.config

all: _openexrid _test _openfx

_openexrid:
	make -C openexrid

_openfx:
	make -C openfx

_test: _openexrid
	make -C test

clean:
	make -C openfx clean
	make -C openexrid clean
	make -C test clean

install:
	make -C openexrid install
	make -C openfx install

dotest: _test _openexrid
	test/$(VERSION)/test
