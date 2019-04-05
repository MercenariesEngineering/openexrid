sinclude makefile.config

all: _openexrid _test _openfx _nuke9 _nuke10 _nuke105 _nuke11 _nuke111 _nuke112 _nuke113

_openexrid:
	make -C openexrid

_openfx:
	make -C openfx

_test: _openexrid
	make -C test

_nuke9:
	make -C nuke9

_nuke10:
	make -C nuke10

_nuke105:
	make -C nuke10.5

_nuke11:
	make -C nuke11

_nuke111:
	make -C nuke11.1

_nuke112:
	make -C nuke11.2

_nuke113:
	make -C nuke11.3

clean:
	make -C openfx clean
	make -C openexrid clean
	make -C nuke9 clean
	make -C nuke10 clean
	make -C nuke10.5 clean
	make -C nuke11 clean
	make -C nuke11.1 clean
	make -C nuke11.2 clean
	make -C nuke11.3 clean
	make -C test clean

install:
	make -C openexrid install
	make -C openfx install
	make -C nuke9 install
	make -C nuke10 install
	make -C nuke10.5 install
	make -C nuke11 install
	make -C nuke11.1 install
	make -C nuke11.2 install
	make -C nuke11.3 install

dotest: _test _openexrid
	test/$(VERSION)/test
