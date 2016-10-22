sinclude ../makefile.myconfig
sinclude ../makefile.config

# Configuration
PREFIX ?= /usr/local/openexrid
EXR_INCLUDE ?= /usr/include/OpenEXR
EXR_LIB ?= /usr/lib

VERSION ?= release
CPPFLAGS += -O3 -Wall -DNDEBUG -fPIC -I $(EXR_INCLUDE)


PWD = $(shell pwd)
SRCS = ${wildcard *.cpp}
OBJS = $(SRCS:%.cpp=$(VERSION)/%.o)

all: $(VERSION) $(VERSION)/test

$(VERSION):	
	mkdir -p $(VERSION)

$(VERSION)/test: $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): $(VERSION)/%.o: %.cpp
	g++ -c $(CPPFLAGS) $< -o $@

clean:
	rm -f $(VERSION)/*.o $(VERSION)/*.a $(VERSION)/*.d $(VERSION)/test

DEPS = $(SRCS:%.cpp=$(VERSION)/%.d)
sinclude $(DEPS)
