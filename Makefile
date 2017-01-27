CC=g++
CFLAGS=-pthread
TEST_CFLAGS=$(CFLAGS) -g -O0

DEP_TARGETS=agent.o control.o

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


# --------------
# Main recipies
# --------------

moe-serifu: $(ODIR)/main.o $(DEP_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)
    
$(ODIR)/main.o: $(ODIR) $(SDIR)/main.cpp $(DEP_INCS)
	$(CC) -c -o $@ $(SDIR)/main.cpp $(CFLAGS)

$(ODIR)/control.o: $(ODIR) $(SDIR)/control.cpp $(SDIR)/control.hpp
	$(CC) -c -o $@ $(SDIR)/control.cpp $(CFLAGS)
    
$(ODIR)/agent.o: $(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(CFLAGS)


# --------------
# Test recipies
# --------------

$(TDIR)/moe-serifu: $(TDIR)/$(ODIR)/main.o $(TEST_DEP_OBJS)
	$(CC) -o $@ $^ $(TEST_CFLAGS)

$(TDIR)/$(ODIR)/main.o: $(TDIR)/$(ODIR) $(SDIR)/main.cpp $(TEST_DEP_INCS)
	$(CC) -c -o $@ $(SDIR)/main.cpp $(TEST_CFLAGS)

$(TDIR)/$(ODIR)/control.o: $(TDIR)/$(ODIR) $(SDIR)/control.cpp $(SDIR)/control.hpp
	$(CC) -c -o $@ $(SDIR)/control.cpp $(TEST_CFLAGS)
    
$(TDIR)/$(ODIR)/agent.o: $(TDIR)/$(ODIR) $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(TEST_CFLAGS)

