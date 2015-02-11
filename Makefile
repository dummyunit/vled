CXX?=g++
CXXFLAGS?=-O0 -ggdb

all: vledd

clean:
	-rm -- Makefile.depend *.o vledd

depend: Makefile.depend

Makefile.depend: *.cc
	$(CXX) $(CXXFLAGS) -std=c++11 -MM $^ > $@

include Makefile.depend

%.o: %.cc
	$(CXX) $(CXXFLAGS) -std=c++11 -c -o $@ $<

vledd: vledd.o virtual_led.o server.o
	$(CXX) $(LDFLAGS) -o $@ $^

.PHONY: all clean depend 
