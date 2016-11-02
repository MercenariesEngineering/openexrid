sinclude ../makefile.myconfig
sinclude ../makefile.config

PWD = $(shell pwd)
SRCS = ${wildcard *.cpp}
OBJS = $(SRCS:%.cpp=$(VERSION)/%.o)

all: $(VERSION) $(VERSION)/DeepOpenEXRId.so

$(VERSION):	
	mkdir -p $(VERSION)

$(VERSION)/DeepOpenEXRId.so: $(OBJS)
	g++ -o $@ $(OBJS) --shared $(LDFLAGS)

$(OBJS): $(VERSION)/%.o: %.cpp
	g++ -c -I.. $(CPPFLAGS) $< -o $@

clean:
	rm -f $(VERSION)/*.o $(VERSION)/*.a $(VERSION)/*.d $(VERSION)/*.ofx

install: $(VERSION)/DeepOpenEXRId.so
	mkdir -p $(PREFIX)/openexrid_nuke_10_linux
	cp $(VERSION)/DeepOpenEXRId.so $(PREFIX)/openexrid_nuke_10_linux
	cp menu.py $(PREFIX)/openexrid_nuke_10_linux

DEPS = $(SRCS:%.cpp=$(VERSION)/%.d)
sinclude $(DEPS)
