CC=g++
CFLAGS=-pthread
ODIR=obj
SDIR=src

moe-serifu: $(ODIR)/msa_core.o $(ODIR)/agent.o $(ODIR)/main.o
	$(CC) -o $@ $^ $(CFLAGS)
    
$(ODIR)/msa_core.o: $(SDIR)/msa_core.cpp $(SDIR)/msa_core.hpp
	$(CC) -c -o $@ $(SDIR)/msa_core.cpp $(CFLAGS)
    
$(ODIR)/agent.o: $(SDIR)/agent.cpp $(SDIR)/agent.hpp
	$(CC) -c -o $@ $(SDIR)/agent.cpp $(CFLAGS)
    
$(ODIR)/main.o: $(SDIR)/main.cpp $(SDIR)/msa_core.hpp
	$(CC) -c -o $@ $(SDIR)/main.cpp $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
