CXXFLAGS += -g -O0 --std=c++11 -I../
VPATH=../:./

all:processdaemon.o
	g++ -o processdaemon $(CXXFLAGS) main.cpp ../processdaemon.cpp

.PHONY:processdaemon.o
processdaemon.o:processdaemon.cpp
	g++ -c -o processdaemon.o $(CXXFLAGS) ../processdaemon.cpp
