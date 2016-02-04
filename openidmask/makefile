sinclude ../makefile.config

PWD = $(shell pwd)
SRCS = ${wildcard *.cpp}
OBJS = $(SRCS:%.cpp=$(VERSION)/%.o)

all: $(VERSION) $(VERSION)/libopenidmask.a

$(VERSION):	
	mkdir -p $(VERSION)

$(VERSION)/libopenidmask.a: $(OBJS)
	ar rvu $(VERSION)/libopenidmask.a $(OBJS)
	ranlib $(VERSION)/libopenidmask.a

$(OBJS): $(VERSION)/%.o: %.cpp
	g++ -c $(CPPFLAGS) $< -o $@

clean:
	rm -f $(VERSION)/*.o $(VERSION)/*.a $(VERSION)/*.d

install: $(VERSION)/libopenidmask.a
	mkdir -p $(PREFIX)/include/openidmask
	mkdir -p $(PREFIX)/lib
	cp *.h $(PREFIX)/include/openidmask
	cp $(VERSION)/libopenidmask.a $(PREFIX)/lib

DEPS = $(SRCS:%.cpp=$(VERSION)/%.d)
sinclude $(DEPS)
