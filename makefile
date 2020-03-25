# make run size=s
# OR
# make run sizeX=x sizeY=y sizeZ=z
# OR
# make analyze file=<infile>

# If only size is defined, then define the individual side
# lengths to be that size.
ifdef size

sizeX=$(size)
sizeY=$(size)
sizeZ=$(size)

endif

sizeString=$(sizeX)_$(sizeY)_$(sizeZ)

ST_ofile=obj/subTree_$(sizeString).o
MC_ofile=obj/monteCarloSearch_$(sizeString)_level$(level).o
GH_ofile=obj/graph_$(sizeString).o

IL_files=indexedList.hpp indexedList.tpp

MC_efile=bin/monteCarloSearch_$(sizeString)_level$(level)

CFLAGS= --std=c++11 -D SIZEX=$(sizeX) -D SIZEY=$(sizeY) -D SIZEZ=$(sizeZ) -pthread -O3

CC=g++-9

all: $(TE_efile)

run: $(TE_efile)
	if [ ! -d results ]; then mkdir results; fi
	./$(TE_efile) results/results_$(sizeString).txt

debug: $(TE_efile)
	if [ ! -d results ]; then mkdir results; fi
	gdb --args ./$(TE_efile) results/results_$(sizeString).txt

perf: $(TE_efile)
	if [ ! -d results ]; then mkdir results; fi
	perf record ./$(TE_efile) results/results_$(sizeString).txt

mcs: $(MC_efile)
	if [ ! -d results ]; then mkdir results; fi
	./$(MC_efile) results/results_$(sizeString).txt

$(MC_efile): $(ST_ofile) $(MC_ofile)
	if [ ! -d obj ]; then mkdir obj; fi
	$(CC) $(CFLAGS) $(ST_ofile) $(MC_ofile) $(GH_ofile) -o $(MC_efile)

$(ST_ofile): $(GH_ofile) subTree.cpp subTree.hpp
	if [ ! -d obj ]; then mkdir obj; fi
	$(CC) $(CFLAGS) -c subTree.cpp -o $(ST_ofile)

$(GH_ofile): graph.cpp graph.hpp
	if [ ! -d obj ]; then mkdir obj; fi
	$(CC) $(CFLAGS) -c graph.cpp -o $(GH_ofile)

$(MC_ofile): monteCarloSearch.cpp $(IL_files)
	if [ ! -d bin ]; then mkdir bin; fi
	$(CC) $(CFLAGS) -D NMC_LEVEL=$(level) -c monteCarloSearch.cpp -o $(MC_ofile)

analyze: bin/analyze
	./bin/analyze < $(file)

bin/analyze: analyzer.cpp
	$(CC) --std=c++11 -O3 analyzer.cpp -o bin/analyze

clean:
	rm -f $(ST_ofile) $(MC_ofile) $(MC_efile)

clean_all:
	rm -f obj/* bin/*
