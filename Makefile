CC=g++
CFLAGS_COMMON=-std=c++11 -pthread
CFLAGS_DEBUG=$(CFLAGS_COMMON) -g -O0
CFLAGS_RELEASE=$(CFLAGS_COMMON)

DEP_TARGETS=agent.o control.o event.o event_handler.o

ODIR=obj
SDIR=src
TDIR=testing

DEP_INCS=$(patsubst %.o,$(SDIR)/%.hpp,$(DEP_TARGETS))
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
	$(CC) -o $@ $^ $(CFLAGS_RELEASE)
    
$(ODIR)/main.o: $(ODIR) $(SDIR)/main.cpp $(DEP_INCS)
	$(CC) -c -o $@ $(SDIR)/main.cpp $(CFLAGS_RELEASE)

$(ODIR)/control.o: $(ODIR) $(SDIR)/control.cpp $(SDIR)/control.hpp $(SDIR)/event_handler.hpp \
 $(SDIR)/environment.hpp
	$(CC) -c -o $@ $(SDIR)/control.cpp $(CFLAGS_RELEASE)
    
$(ODIR)/agent.o: $(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(CFLAGS_RELEASE)

$(ODIR)/event.o: $(ODIR) $(SDIR)/event.cpp $(SDIR)/event.hpp
	$(CC) -c -o $@ $(SDIR)/event.cpp $(CFLAGS_RELEASE)

$(ODIR)/event_handler.o: $(ODIR) $(SDIR)/event_handler.cpp $(SDIR)/event_handler.hpp $(SDIR)/event.hpp
	$(CC) -c -o $@ $(SDIR)/event_handler.cpp $(CFLAGS_RELEASE)



# ---------------
# Debug recipies
# ---------------

$(TDIR)/moe-serifu: $(TDIR)/$(ODIR)/main.o $(DEP_OBJS_DEBUG)
	$(CC) -o $@ $^ $(CFLAGS_DEBUG)

$(TDIR)/$(ODIR)/main.o: $(TDIR)/$(ODIR) $(SDIR)/main.cpp $(DEP_INCS)
	$(CC) -c -o $@ $(SDIR)/main.cpp $(CFLAGS_DEBUG)

$(TDIR)/$(ODIR)/control.o: $(TDIR)/$(ODIR) $(SDIR)/control.cpp $(SDIR)/control.hpp $(SDIR)/event_handler.hpp \
 $(SDIR)/environment.hpp
	$(CC) -c -o $@ $(SDIR)/control.cpp $(CFLAGS_DEBUG)
    
$(TDIR)/$(ODIR)/agent.o: $(TDIR)/$(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(CFLAGS_DEBUG)

$(TDIR)/$(ODIR)/event.o: $(TDIR)/$(ODIR) $(SDIR)/event.cpp $(SDIR)/event.hpp
	$(CC) -c -o $@ $(SDIR)/event.cpp $(CFLAGS_DEBUG)

$(TDIR)/$(ODIR)/event_handler.o: $(TDIR)/$(ODIR) $(SDIR)/event_handler.cpp $(SDIR)/event_handler.hpp $(SDIR)/event.hpp
	$(CC) -c -o $@ $(SDIR)/event_handler.cpp $(CFLAGS_DEBUG)


