ODIR ?= obj
SDIR ?= src
OS_SDIR ?= compat
PLDIR ?= plugins

INCLUDE_DIRS = -I$(SDIR) -I$(OS_SDIR)

CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Wpedantic -pthread $(INCLUDE_DIRS) -include compat/compat.hpp
LDFLAGS ?= -ldl -lpthread

DEP_TARGETS ?= agent/agent.o util.o msa.o event/event.o event/handler.o event/dispatch.o input/input.o string.o configuration.o cmd.o log.o output.o var.o plugin.o
DEP_INCS = $(patsubst %.o,$(SDIR)/%.hpp,$(DEP_TARGETS))
DEP_OBJS = $(patsubst %,$(ODIR)/%,$(DEP_TARGETS))
DEP_SOURCES = $(patsubst %.o,%.cpp,$(DEP_TARGETS))
DEP_EXS = $(SDIR)/debug_macros.hpp

OS_DEP_TARGETS = thread/thread.o file/file.o lib/lib.o
OS_DEP_OBJS = $(patsubst %,$(ODIR)/platform/%,$(notdir $(OS_DEP_TARGETS)))
OS_DEP_SOURCES = $(patsubst %.o,platform/%.cpp,$(OS_DEP_TARGETS))

.PHONY: clean test all debug plugins clean-plugins gen-deps

all: moe-serifu plugins

debug: CXXFLAGS += -g -O0 -Werror -DDEBUG
debug: moe-serifu

test: debug
	valgrind --leak-check=yes --track-origins=yes moe-serifu

clean: clean-plugins
	rm -f $(ODIR)/*.o
	rm -f $(ODIR)/event/*.o
	rm -f $(ODIR)/platform/*.o
	rm -f $(ODIR)/agent/*.o
	rm -f $(ODIR)/input/*.o
	rm -f moe-serifu

gen-deps:
	scripts/gendeps.py SDIR $(SDIR) $(INCLUDE_DIRS) $(patsubst %,-E%,$(DEP_EXS)) $(DEP_SOURCES) > scripts/modules.mk
	scripts/gendeps.py OS_SDIR $(OS_SDIR) $(INCLUDE_DIRS) $(OS_DEP_SOURCES) -c1 > scripts/os_modules.mk

include scripts/*.mk


# ----------------- #
#  Binary Recipies  #
# ----------------- #

moe-serifu: $(ODIR)/main.o $(DEP_OBJS) $(OS_DEP_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(ODIR)/main.o: $(SDIR)/main.cpp $(DEP_INCS)
	$(CXX) -c -o $@ $(SDIR)/main.cpp $(CXXFLAGS)


# --------- #
#  Plugins  #
# --------- #

clean-plugins:	
	rm -f $(PLDIR)/autoload/*.so
	rm -f $(PLDIR)/example/*.so
	rm -f $(PLDIR)/example/*.o

plugins: $(PLDIR)/autoload/example.so

$(PLDIR)/autoload/example.so: $(PLDIR)/example/example.so
	cp $(PLDIR)/example/example.so $(PLDIR)/autoload/example.so

$(PLDIR)/example/example.so: $(PLDIR)/example/example.o
	$(CXX) -o $@ $(PLDIR)/example/example.o -shared
	
$(PLDIR)/example/example.o: $(PLDIR)/example/example.cpp $(SDIR)/plugin.hpp
	$(CXX) -c -o $@ $(PLDIR)/example/example.cpp -I$(SDIR) -include compat/compat.hpp -fPIC -std=c++11 -Wall -Wextra -Wpedantic

