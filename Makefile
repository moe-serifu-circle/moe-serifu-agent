ODIR ?= obj
SDIR ?= src
OS_SDIR ?= compat
PLDIR ?= plugins

INCLUDE_DIRS = -I$(SDIR) -I$(OS_SDIR)

# python interpreter is normally set by the shebang of the scripts, but some environments don't support standard
# shebangs, and should export this variable before running scripts:
PYTHON ?=

CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Wpedantic -pthread $(INCLUDE_DIRS) -include compat/compat.hpp
LDFLAGS ?= -ldl -lpthread

DEP_TARGETS ?= agent/agent.o util.o msa.o event/event.o event/handler.o event/dispatch.o input/input.o string.o cfg/cfg.o cmd/cmd.o log/log.o output/output.o var.o plugin/plugin.o
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
	rm -f $(patsubst %,$(ODIR)/%*.o,$(sort $(subst ./,,$(dir $(DEP_TARGETS)))))
	rm -f $(ODIR)/platform/*.o
	rm -f moe-serifu

gen-deps:
	$(PYTHON) scripts/gendeps.py SDIR $(SDIR) $(INCLUDE_DIRS) $(patsubst %,-E%,$(DEP_EXS)) $(DEP_SOURCES) > scripts/modules.mk
	$(PYTHON) scripts/gendeps.py OS_SDIR $(OS_SDIR) $(INCLUDE_DIRS) $(OS_DEP_SOURCES) -c1 > scripts/os_modules.mk

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
	
$(PLDIR)/example/example.o: $(PLDIR)/example/example.cpp $(SDIR)/plugin/plugin.hpp
	$(CXX) -c -o $@ $(PLDIR)/example/example.cpp -I$(SDIR) -include compat/compat.hpp -fPIC -std=c++11 -Wall -Wextra -Wpedantic

