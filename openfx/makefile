sinclude ../makefile.config

CPPFLAGS += -I ..
PWD = $(shell pwd)
SRCS = ${wildcard *.cpp}
OBJS = $(SRCS:%.cpp=$(VERSION)/%.o)

all: $(VERSION) $(VERSION)/openexrid.ofx

$(VERSION):	
	mkdir -p $(VERSION)

$(VERSION)/openexrid.ofx: $(OBJS)
	g++ -o $@ $(OBJS) --shared $(LDFLAGS)

$(OBJS): $(VERSION)/%.o: %.cpp
	g++ -c $(CPPFLAGS) $< -o $@

clean:
	rm -f $(VERSION)/*.o $(VERSION)/*.a $(VERSION)/*.d

install: $(VERSION)/openexrid.ofx
	mkdir -p $(PREFIX)/openexrid.ofx.bundle/Contents/Linux-x86-64
	cp $(VERSION)/openexrid.ofx $(PREFIX)/openexrid.ofx.bundle/Contents/Linux-x86-64

DEPS = $(SRCS:%.cpp=$(VERSION)/%.d)
sinclude $(DEPS)
