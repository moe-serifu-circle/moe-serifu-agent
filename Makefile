ODIR?=obj
SDIR?=src
TDIR?=testing

CXX?=g++
CXXFLAGS?=-std=c++11 -Wall -Wextra -Wpedantic -Werror -pthread -I$(SDIR)
CXXFLAGS_DEBUG=$(CXXFLAGS) -g -O0
CXXFLAGS_RELEASE=$(CXXFLAGS)

DEP_TARGETS?=agent.o util.o msa.o event_event.o event_handler.o event_dispatch.o input.o
DEP_INC_PATHS?=agent.hpp util.hpp msa.hpp event/event.hpp event/handler.hpp event/dispatch.hpp input.hpp
DEP_INCS=$(patsubst %,$(SDIR)/%,$(DEP_INC_PATHS))

DEP_OBJS=$(patsubst %,$(ODIR)/%,$(DEP_TARGETS))
DEP_OBJS_DEBUG=$(patsubst %,$(TDIR)/$(ODIR)/%,$(DEP_TARGETS))

.PHONY: clean directories test

all: directories moe-serifu

directories: $(ODIR)

test: $(TDIR)/moe-serifu
	valgrind --leak-check=yes --track-origins=yes $(TDIR)/moe-serifu

clean:
	rm -f $(ODIR)/*.o
	rm -rf $(ODIR)
	rm -f $(TDIR)/moe-serifu
	rm -f $(TDIR)/$(ODIR)/*.o
	rm -rf $(TDIR)/$(ODIR)
	rm -rf $(TDIR)
	rm -f moe-serifu


# -------------
# Dir recipies
# -------------

$(ODIR):
	mkdir -p $(ODIR)

$(TDIR):
	mkdir -p $(TDIR)

$(TDIR)/$(ODIR):
	mkdir -p $(TDIR)/$(ODIR)


# -----------------
# Release recipies
# -----------------

moe-serifu: $(ODIR)/main.o $(DEP_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS_RELEASE)

$(ODIR)/main.o: $(ODIR) $(SDIR)/main.cpp $(DEP_INCS)
	$(CXX) -c -o $@ $(SDIR)/main.cpp $(CXXFLAGS_RELEASE)

$(ODIR)/msa.o: $(ODIR) $(SDIR)/msa.cpp $(SDIR)/msa.hpp $(SDIR)/input.hpp $(SDIR)/event/dispatch.hpp $(SDIR)/agent.hpp
	$(CXX) -c -o $@ $(SDIR)/msa.cpp $(CXXFLAGS_RELEASE)

$(ODIR)/agent.o: $(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp $(SDIR)/msa.hpp
	$(CXX) -c -o $@ $(SDIR)/agent.cpp $(CXXFLAGS_RELEASE)

$(ODIR)/util.o: $(ODIR) $(SDIR)/util.cpp $(SDIR)/util.hpp
	$(CXX) -c -o $@ $(SDIR)/util.cpp $(CXXFLAGS_RELEASE)

$(ODIR)/input.o: $(ODIR) $(SDIR)/input.cpp $(SDIR)/input.hpp $(SDIR)/msa.hpp $(SDIR)/event/dispatch.hpp
	$(CXX) -c -o $@ $(SDIR)/input.cpp $(CXXFLAGS_RELEASE)

$(ODIR)/event_handler.o: $(ODIR) $(SDIR)/event/handler.cpp $(SDIR)/msa.hpp $(SDIR)/event/event.hpp
	$(CXX) -c -o $@ $(SDIR)/event/handler.cpp $(CXXFLAGS_RELEASE)

$(ODIR)/event_event.o: $(ODIR) $(SDIR)/event/event.cpp $(SDIR)/event/event.hpp
	$(CXX) -c -o $@ $(SDIR)/event/event.cpp $(CXXFLAGS_RELEASE)

$(ODIR)/event_dispatch.o: $(ODIR) $(SDIR)/event/dispatch.cpp $(SDIR)/msa.hpp $(SDIR)/event/handler.hpp $(SDIR)/util.hpp
	$(CXX) -c -o $@ $(SDIR)/event/dispatch.cpp $(CXXFLAGS_RELEASE)


# -----------------
# Debug recipies
# -----------------

$(TDIR)/moe-serifu: $(TDIR)/$(ODIR)/main.o $(DEP_OBJS_DEBUG)
	$(CXX) -o $@ $^ $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/main.o: $(TDIR)/$(ODIR) $(SDIR)/main.cpp $(DEP_INCS)
	$(CXX) -c -o $@ $(SDIR)/main.cpp $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/msa.o: $(TDIR)/$(ODIR) $(SDIR)/msa.cpp $(SDIR)/msa.hpp $(SDIR)/input.hpp $(SDIR)/event/dispatch.hpp $(SDIR)/agent.hpp
	$(CXX) -c -o $@ $(SDIR)/msa.cpp $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/agent.o: $(TDIR)/$(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp $(SDIR)/msa.hpp
	$(CXX) -c -o $@ $(SDIR)/agent.cpp $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/util.o: $(TDIR)/$(ODIR) $(SDIR)/util.cpp $(SDIR)/util.hpp
	$(CXX) -c -o $@ $(SDIR)/util.cpp $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/input.o: $(TDIR)/$(ODIR) $(SDIR)/input.cpp $(SDIR)/input.hpp $(SDIR)/msa.hpp $(SDIR)/event/dispatch.hpp
	$(CXX) -c -o $@ $(SDIR)/input.cpp $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/event_handler.o: $(TDIR)/$(ODIR) $(SDIR)/event/handler.cpp $(SDIR)/msa.hpp $(SDIR)/event/event.hpp
	$(CXX) -c -o $@ $(SDIR)/event/handler.cpp $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/event_event.o: $(TDIR)/$(ODIR) $(SDIR)/event/event.cpp $(SDIR)/event/event.hpp
	$(CXX) -c -o $@ $(SDIR)/event/event.cpp $(CXXFLAGS_DEBUG)

$(TDIR)/$(ODIR)/event_dispatch.o: $(ODIR) $(SDIR)/event/dispatch.cpp $(SDIR)/msa.hpp $(SDIR)/event/handler.hpp $(SDIR)/util.hpp
	$(CXX) -c -o $@ $(SDIR)/event/dispatch.cpp $(CXXFLAGS_DEBUG)
