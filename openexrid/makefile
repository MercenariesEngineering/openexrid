sinclude ../makefile.myconfig
sinclude ../makefile.config

PWD = $(shell pwd)
SRCS = ${wildcard *.cpp}
OBJS = $(SRCS:%.cpp=$(VERSION)/%.o)

all: $(VERSION) $(VERSION)/libopenexrid.a

$(VERSION):	
	mkdir -p $(VERSION)

$(VERSION)/libopenexrid.a: $(OBJS)
	ar rvu $(VERSION)/libopenexrid.a $(OBJS)
	ranlib $(VERSION)/libopenexrid.a

$(OBJS): $(VERSION)/%.o: %.cpp
	g++ -c $(CPPFLAGS) $< -o $@

clean:
	rm -f $(VERSION)/*.o $(VERSION)/*.a $(VERSION)/*.d

install: $(VERSION)/libopenexrid.a
	mkdir -p $(PREFIX)/include/openexrid
	mkdir -p $(PREFIX)/lib
	cp *.h $(PREFIX)/include/openexrid
	cp $(VERSION)/libopenexrid.a $(PREFIX)/lib

DEPS = $(SRCS:%.cpp=$(VERSION)/%.d)
sinclude $(DEPS)
