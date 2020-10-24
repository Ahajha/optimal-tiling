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

CC          = g++-10 --std=c++20
CFLAGS      = -pthread -O3 -Wall -Wextra
SIZE_MACRO  = -D SIZEX=$(sizeX) -D SIZEY=$(sizeY) -D SIZEZ=$(sizeZ)
LEVEL_MACRO = -D NMC_LEVEL=$(level)

sizeString=$(sizeX)_$(sizeY)_$(sizeZ)

ST_ofile=obj/subTree_$(sizeString).o
MC_ofile=obj/monteCarloSearch_$(sizeString)_level$(level).o
TE_ofile=obj/treeEnumerator_$(sizeString).o
GH_ofile=obj/graph_$(sizeString).o
DF_ofile=obj/defs_$(sizeString).o

IL_files=src/indexedList.hpp src/indexedList.tpp

MC_efile=bin/monteCarloSearch_$(sizeString)_level$(level)
TE_efile=bin/treeEnumerator_$(sizeString)

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

analyze: bin/analyze
	./bin/analyze < $(file)

$(MC_efile): $(MC_ofile) $(ST_ofile) $(GH_ofile) $(DF_ofile)
$(TE_efile): $(TE_ofile) $(ST_ofile) $(GH_ofile) $(DF_ofile)
bin/analyze: src/analyzer.cpp

bin/%:
	$(CC) $(CFLAGS) $^ -o $@

$(MC_ofile): src/monteCarloSearch.cpp $(IL_files) src/defs.hpp
	$(CC) $(CFLAGS) $(SIZE_MACRO) $(LEVEL_MACRO) -c $< -o $@

$(ST_ofile): src/subTree.cpp src/subTree.hpp src/graph.hpp src/defs.hpp
$(GH_ofile): src/graph.cpp src/graph.hpp src/defs.hpp
$(DF_ofile): src/defs.cpp src/defs.hpp src/subTree.hpp src/graph.hpp
$(TE_ofile): src/treeEnumerator.cpp $(IL_files)

obj/%:
	$(CC) $(CFLAGS) $(SIZE_MACRO) -c $< -o $@

clean:
	rm -f obj/* bin/*
