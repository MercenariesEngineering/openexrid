sinclude ../makefile.myconfig
sinclude ../makefile.config

PWD = $(shell pwd)
SRCS := $(shell ls ../nuke/*.cpp)
OBJS = $(addprefix $(VERSION)/,$(addsuffix .o,$(basename $(notdir $(SRCS)))))
SRCS_OSL := $(shell ls ../nuke/OSL/*.cpp)
OBJS_OSL = $(addprefix $(VERSION)/,$(addsuffix .o,$(basename $(notdir $(SRCS_OSL)))))

all: $(VERSION) $(VERSION)/DeepOpenEXRId.so

$(VERSION):	
	mkdir -p $(VERSION)

$(VERSION)/DeepOpenEXRId.so: $(OBJS) $(OBJS_OSL)
	$(CXX) -o $@ $(OBJS) $(OBJS_OSL) --shared $(LDFLAGS)

$(OBJS): $(VERSION)/%.o: $(PWD)/../nuke/%.cpp
	$(CXX) -c -I.. $(CPPFLAGS) -I $(NUKE_INCLUDE) -I ../nuke/OSL/include $< -o $@

$(OBJS_OSL): $(VERSION)/%.o: $(PWD)/../nuke/OSL/%.cpp
	$(CXX) -c -I.. $(CPPFLAGS) -I $(NUKE_INCLUDE) -I ../nuke/OSL/include $< -o $@

clean:
	rm -f $(VERSION)/*.o $(VERSION)/*.a $(VERSION)/*.d $(VERSION)/*.ofx

install: $(VERSION)/DeepOpenEXRId.so
	mkdir -p $(PREFIX)/$(NUKE_VERSION)
	cp $(VERSION)/DeepOpenEXRId.so $(PREFIX)/$(NUKE_VERSION)/
	strip $(PREFIX)/$(NUKE_VERSION)/DeepOpenEXRId.so
	cp $(VERSION)/DeepOpenEXRId.so ~/.nuke/
	cp ../nuke/menu.py ~/.nuke/

DEPS = $(SRCS:%.cpp=$(VERSION)/%.d)
sinclude $(DEPS)
