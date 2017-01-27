CC=g++
CFLAGS=-pthread
ODIR=obj
SDIR=src
MKDIR_P=mkdir -p

.PHONY: clean directories

all: directories moe-serifu

directories: $(ODIR)

$(ODIR):
	${MKDIR_P} $(ODIR)

moe-serifu: $(ODIR)/msa_core.o $(ODIR)/agent.o $(ODIR)/main.o
	$(CC) -o $@ $^ $(CFLAGS)
    
$(ODIR)/msa_core.o: $(SDIR)/msa_core.cpp $(SDIR)/msa_core.hpp
	$(CC) -c -o $@ $(SDIR)/msa_core.cpp $(CFLAGS)
    
$(ODIR)/agent.o: $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(CFLAGS)
    
$(ODIR)/main.o: $(SDIR)/main.cpp $(SDIR)/msa_core.hpp
	$(CC) -c -o $@ $(SDIR)/main.cpp $(CFLAGS)

clean:
	rm -rf $(ODIR)/*.o $(ODIR) moe-serifu
