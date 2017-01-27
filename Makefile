CC=g++
CFLAGS=-pthread
TEST_CFLAGS=$(CFLAGS) -g -O0

DEP_TARGETS=agent.o msa_core.o

ODIR=obj
SDIR=src
TDIR=testing

DEP_INCS=$(patsubst %.o,$(SDIR)/%.hpp,$(DEP_TARGETS))
DEP_OBJS=$(patsubst %,$(ODIR)/%,$(DEP_TARGETS))
TEST_DEP_OBJS=$(patsubst %,$(TDIR)/$(ODIR)/%,$(DEP_TARGETS))



.PHONY: clean directories test

all: directories moe-serifu

directories: $(ODIR)

test: clean $(TDIR)/moe-serifu
	valgrind --leak-check=yes --track-origins=yes $(TDIR)/moe-serifu

clean:
	rm -f $(ODIR)/*.o
	rm -r $(ODIR)
	rm -f $(TDIR)/moe-serifu
	rm -f $(TDIR)/$(ODIR)/*.o
	rm -r $(TDIR)/$(ODIR)
	rm -r $(TDIR)
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


# --------------
# Main recipies
# --------------

moe-serifu: $(ODIR)/main.o $(DEP_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR)/msa_core.o: $(ODIR) $(SDIR)/msa_core.cpp $(SDIR)/msa_core.hpp
	$(CC) -c -o $@ $(SDIR)/msa_core.cpp $(CFLAGS)
    
$(ODIR)/agent.o: $(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(CFLAGS)
    
$(ODIR)/main.o: $(ODIR) $(SDIR)/main.cpp $(DEP_INCS)
	$(CC) -c -o $@ $(SDIR)/main.cpp $(CFLAGS)


# --------------
# Test recipies
# --------------

$(TDIR)/moe-serifu: $(TDIR) $(TDIR)/$(ODIR)/main.o $(TEST_DEP_OBJS)
	$(CC) -o $@ $^ $(TEST_CFLAGS)

$(TDIR)/$(ODIR)/main.o: $(TDIR)/$(ODIR) $(SDIR)/main.cpp $(TEST_DEP_INCS)
	$(CC) -c -o $@ $(SDIR)/main.cpp $(TEST_CFLAGS)

$(TDIR)/$(ODIR)/msa_core.o: $(TDIR)/$(ODIR) $(SDIR)/msa_core.cpp $(SDIR)/msa_core.hpp
	$(CC) -c -o $@ $(SDIR)/msa_core.cpp $(TEST_CFLAGS)
    
$(TDIR)/$(ODIR)/agent.o: $(TDIR)/$(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(TEST_CFLAGS)