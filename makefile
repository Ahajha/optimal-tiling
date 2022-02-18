$(shell mkdir -p obj bin results)

comma = ,
sizeString = $(subst $(comma),_,$(size))

CC=g++-10
CFLAGS=--std=c++20 -O3 -g -Wall -Wextra -Wpedantic -I cpp-equiv/include -pthread
SIZE_MACRO  = -D SIZE=$(size)
LEVEL_MACRO = -D NMC_LEVEL=$(level)

PERMUTATION=src/permutation.hpp src/permutation.tpp
SLICE=src/slice.hpp src/slice.tpp $(PERMUTATION)

ST_ofile=obj/subTree_$(sizeString).o
MC_ofile=obj/monteCarloSearch_$(sizeString)_level$(level).o
TE_ofile=obj/treeEnumerator_$(sizeString).o
GH_ofile=obj/graph_$(sizeString).o
DF_ofile=obj/defs_$(sizeString).o

IL_files=src/indexedList.hpp src/indexedList.tpp

MC_efile=bin/monteCarloSearch_$(sizeString)_level$(level)
TE_efile=bin/treeEnumerator_$(sizeString)

help:
	@echo "make tile size=A,B,C,... : run the optimal tiling algorithm on a given size"
	@echo "make cube size=A,B,C,... : run the optimal cube algorithm on a given size"
	@echo "make all  size=A,B,C,... : compile all programs without running"
	@echo ""
	@echo "For performance concerns, it is suggested that the dimension sizes be given\
	 in nonascending order."

all: bin/optimal_tile_$(sizeString) bin/optimal_cube_$(sizeString) $(MC_efile) $(TE_efile)

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

tile: bin/optimal_tile_$(sizeString)
	./bin/optimal_tile_$(sizeString)

cube: bin/optimal_cube_$(sizeString)
	./bin/optimal_cube_$(sizeString)

debug_tile: bin/optimal_tile_$(sizeString)
	gdb ./bin/optimal_tile_$(sizeString)

debug_cube: bin/optimal_cube_$(sizeString)
	gdb ./bin/optimal_cube_$(sizeString)

perf_tile: bin/optimal_tile_$(sizeString)
	perf record ./bin/optimal_tile_$(sizeString)

perf_cube: bin/optimal_cube_$(sizeString)
	perf record ./bin/optimal_cube_$(sizeString)

obj/optimal_tile_$(sizeString).o: src/optimal_tile.cpp $(SLICE)
obj/optimal_cube_$(sizeString).o: src/optimal_cube.cpp $(SLICE)

bin/optimal_tile_$(sizeString): obj/optimal_tile_$(sizeString).o
bin/optimal_cube_$(sizeString): obj/optimal_cube_$(sizeString).o

$(MC_ofile): src/monteCarloSearch.cpp $(IL_files) src/defs.hpp
	$(CC) $(CFLAGS) $(SIZE_MACRO) $(LEVEL_MACRO) -c $< -o $@

$(ST_ofile): src/subTree.cpp src/subTree.hpp src/graph.hpp src/defs.hpp
$(GH_ofile): src/graph.cpp src/graph.hpp src/defs.hpp
$(DF_ofile): src/defs.cpp src/defs.hpp src/subTree.hpp src/graph.hpp
$(TE_ofile): src/treeEnumerator.cpp $(IL_files)

# TODO We only need one or the other macro
obj/%:
	$(CC) $(CFLAGS) $(SIZE_MACRO) -DDIM_SIZES=$(size) -c $< -o $@

bin/%:
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f obj/* bin/*
