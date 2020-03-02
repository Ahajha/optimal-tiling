# This is a 'default' of sorts, for testing.
# Use make run size=x for any other size
size=4

ST_ofile=obj/subTree_$(size).o
TE_ofile=obj/treeEnumerator_$(size).o

TE_efile=bin/treeEnumerator_$(size)

all: $(TE_efile)

run: $(TE_efile)
	./$(TE_efile) results/results_$(size).txt

$(TE_efile): $(ST_ofile) $(TE_ofile)
	g++ -DSIZE=$(size) -pthread -O3 $(ST_ofile) $(TE_ofile) -o $(TE_efile)

$(ST_ofile): subTree.cpp subTree.hpp
	g++ -DSIZE=$(size) -pthread -O3 -c subTree.cpp -o $(ST_ofile)

$(TE_ofile): treeEnumerator_threaded.cpp
	g++ -DSIZE=$(size) -pthread -O3 -c treeEnumerator_threaded.cpp -o $(TE_ofile)

clean:
	rm -f $(ST_ofile) $(TE_ofile) $(TE_efile)

clean_all:
	rm -f obj/* bin/*
