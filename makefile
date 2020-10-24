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

$(shell mkdir -p obj bin results)

sizeString=$(sizeX)_$(sizeY)_$(sizeZ)

ST_ofile=obj/subTree_$(sizeString).o
MC_ofile=obj/monteCarloSearch_$(sizeString)_level$(level).o
TE_ofile=obj/treeEnumerator_$(sizeString).o
GH_ofile=obj/graph_$(sizeString).o
DF_ofile=obj/defs_$(sizeString).o

IL_files=indexedList.hpp indexedList.tpp

MC_efile=bin/monteCarloSearch_$(sizeString)_level$(level)
TE_efile=bin/treeEnumerator_$(sizeString)

CFLAGS= --std=c++20 -D SIZEX=$(sizeX) -D SIZEY=$(sizeY) -D SIZEZ=$(sizeZ) -pthread -O3 -Wall -Wextra

CC=g++-10

all: $(MC_efile) $(TE_efile)

run: $(TE_efile)
	./$(TE_efile) results/results_$(sizeString).txt

debug_run: $(TE_efile)
	gdb --args ./$(TE_efile) results/results_$(sizeString).txt

perf_run: $(TE_efile)
	perf record ./$(TE_efile) results/results_$(sizeString).txt

mcs: $(MC_efile)
	./$(MC_efile) results/results_$(sizeString).txt

debug_mcs: $(MC_efile)
	gdb --args ./$(MC_efile) results/results_$(sizeString).txt

perf_mcs: $(MC_efile)
	perf record ./$(MC_efile) results/results_$(sizeString).txt

$(MC_efile): $(ST_ofile) $(MC_ofile) $(GH_ofile) $(DF_ofile)
	$(CC) $(CFLAGS) $(ST_ofile) $(MC_ofile) $(GH_ofile) $(DF_ofile) -o $(MC_efile)

$(TE_efile): $(ST_ofile) $(TE_ofile) $(GH_ofile) $(DF_ofile)
	$(CC) $(CFLAGS) $(ST_ofile) $(TE_ofile) $(GH_ofile) $(DF_ofile) -o $(TE_efile)

$(ST_ofile): $(GH_ofile) subTree.cpp subTree.hpp
	$(CC) $(CFLAGS) -c subTree.cpp -o $(ST_ofile)

$(GH_ofile): graph.cpp graph.hpp
	$(CC) $(CFLAGS) -c graph.cpp -o $(GH_ofile)

$(DF_ofile): defs.hpp defs.cpp
	$(CC) $(CFLAGS) -c defs.cpp -o $(DF_ofile)

$(TE_ofile): treeEnumerator.cpp $(IL_files)
	$(CC) $(CFLAGS) -c treeEnumerator.cpp -o $(TE_ofile)

$(MC_ofile): monteCarloSearch.cpp $(IL_files) defs.hpp
	$(CC) $(CFLAGS) -D NMC_LEVEL=$(level) -c monteCarloSearch.cpp -o $(MC_ofile)

analyze: bin/analyze
	./bin/analyze < $(file)

bin/analyze: analyzer.cpp
	$(CC) --std=c++11 -O3 analyzer.cpp -o bin/analyze

clean:
	rm -f obj/* bin/*
