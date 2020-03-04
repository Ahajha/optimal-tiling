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
TE_ofile=obj/treeEnumerator_$(sizeString).o

TE_efile=bin/treeEnumerator_$(sizeString)

CFLAGS= --std=c++11 -D SIZEX=$(sizeX) -D SIZEY=$(sizeY) -D SIZEZ=$(sizeZ) -pthread -O3

all: $(TE_efile)

run: $(TE_efile)
	if [ ! -d results ]; then mkdir results; fi
	./$(TE_efile) results/results_$(sizeString).txt

$(TE_efile): $(ST_ofile) $(TE_ofile)
	if [ ! -d obj ]; then mkdir obj; fi
	g++ $(CFLAGS) $(ST_ofile) $(TE_ofile) -o $(TE_efile)

$(ST_ofile): subTree.cpp subTree.hpp
	if [ ! -d obj ]; then mkdir obj; fi
	g++ $(CFLAGS) -c subTree.cpp -o $(ST_ofile)

$(TE_ofile): treeEnumerator_threaded.cpp
	if [ ! -d bin ]; then mkdir bin; fi
	g++ $(CFLAGS) -c treeEnumerator_threaded.cpp -o $(TE_ofile)

analyze: bin/analyze
	./analyze $(file)

bin/analyze: analyzer.cpp
	g++ --std=c++11 -O3 analyzer.cpp -o /bin/analyzer

clean:
	rm -f $(ST_ofile) $(TE_ofile) $(TE_efile)

clean_all:
	rm -f obj/* bin/*
