ODIR?=obj
SDIR?=src

CXX?=g++
CXXFLAGS?=-std=c++11 -Wall -Wextra -Wpedantic -pthread -I$(SDIR) -Icompat -include compat/compat.hpp

DEP_TARGETS?=agent.o util.o msa.o event/event.o event/handler.o event/dispatch.o input.o string.o configuration.o cmd.o log.o
DEP_INCS=$(patsubst %.o,$(SDIR)/%.hpp,$(DEP_TARGETS))
DEP_OBJS=$(patsubst %,$(ODIR)/%,$(DEP_TARGETS))

OS_DEP_TARGETS=thread.o
OS_DEP_OBJS=$(ODIR)/platform/thread.o

.PHONY: clean test all debug

all: moe-serifu

debug: CXXFLAGS += -g -O0 -Werror
debug: moe-serifu

test: debug
	valgrind --leak-check=yes --track-origins=yes moe-serifu

clean:
	rm -f $(ODIR)/*.o
	rm -f $(ODIR)/event/*.o
	rm -f $(ODIR)/platform/*.o
	rm -f moe-serifu


# ----------------- #
#  Binary Recipies  #
# ----------------- #

moe-serifu: $(ODIR)/main.o $(DEP_OBJS) $(OS_DEP_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(ODIR)/main.o: $(SDIR)/main.cpp $(DEP_INCS)
	$(CXX) -c -o $@ $(SDIR)/main.cpp $(CXXFLAGS)

$(ODIR)/msa.o: $(SDIR)/msa.cpp $(SDIR)/msa.hpp $(SDIR)/input.hpp $(SDIR)/event/dispatch.hpp $(SDIR)/agent.hpp $(SDIR)/configuration.hpp $(SDIR)/cmd.hpp
	$(CXX) -c -o $@ $(SDIR)/msa.cpp $(CXXFLAGS)

$(ODIR)/configuration.o: $(SDIR)/configuration.cpp $(SDIR)/configuration.hpp $(SDIR)/string.hpp
	$(CXX) -c -o $@ $(SDIR)/configuration.cpp $(CXXFLAGS)

$(ODIR)/string.o: $(SDIR)/string.cpp $(SDIR)/string.hpp
	$(CXX) -c -o $@ $(SDIR)/string.cpp $(CXXFLAGS)

$(ODIR)/agent.o: $(SDIR)/agent.cpp $(SDIR)/agent.hpp $(SDIR)/msa.hpp $(SDIR)/configuration.hpp
	$(CXX) -c -o $@ $(SDIR)/agent.cpp $(CXXFLAGS)

$(ODIR)/util.o: $(SDIR)/util.cpp $(SDIR)/util.hpp
	$(CXX) -c -o $@ $(SDIR)/util.cpp $(CXXFLAGS)

$(ODIR)/input.o: $(SDIR)/input.cpp $(SDIR)/input.hpp $(SDIR)/msa.hpp $(SDIR)/event/dispatch.hpp  $(SDIR)/configuration.hpp
	$(CXX) -c -o $@ $(SDIR)/input.cpp $(CXXFLAGS)

$(ODIR)/event/handler.o: $(SDIR)/event/handler.cpp $(SDIR)/msa.hpp $(SDIR)/event/event.hpp
	$(CXX) -c -o $@ $(SDIR)/event/handler.cpp $(CXXFLAGS)

$(ODIR)/event/event.o: $(SDIR)/event/event.cpp $(SDIR)/event/event.hpp
	$(CXX) -c -o $@ $(SDIR)/event/event.cpp $(CXXFLAGS)

$(ODIR)/event/dispatch.o: $(SDIR)/event/dispatch.cpp $(SDIR)/msa.hpp $(SDIR)/event/handler.hpp $(SDIR)/util.hpp $(SDIR)/configuration.hpp
	$(CXX) -c -o $@ $(SDIR)/event/dispatch.cpp $(CXXFLAGS)

$(ODIR)/cmd.o: $(SDIR)/cmd.cpp $(SDIR)/cmd.hpp $(SDIR)/event/dispatch.hpp
	$(CXX) -c -o $@ $(SDIR)/cmd.cpp $(CXXFLAGS)

$(ODIR)/log.o: $(SDIR)/log.cpp $(SDIR)/log.hpp $(SDIR)/configuration.hpp $(SDIR)/string.hpp $(SDIR)/util.hpp
	$(CXX) -c -o $@ $(SDIR)/log.cpp $(CXXFLAGS)


# ------------------- #
#  OS Binary Recipes  #
# ------------------- #

$(ODIR)/platform/thread.o: compat/platform/thread/thread.cpp
	$(CXX) -c -o $@ compat/platform/thread/thread.cpp

