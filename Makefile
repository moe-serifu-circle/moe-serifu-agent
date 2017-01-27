IDIR=include
CC=gcc
CFLAGS=-I$(IDIR) -pthread

ODIR=obj
LDIR=lib

LIBS=

moe-serifu: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
    
$(ODIR)/msa_core.o: msa_core.cpp msa_core.hpp
	$(CC) -c -o $@ $(CFLAGS)
    
$(ODIR)/agent.o: agent.cpp agent.hpp
	$(CC) -c -o $@ $(CFLAGS)
    
$(ODIR)/main.o: main.cpp msa_core.hpp
	$(CC) -c -o $@ $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o