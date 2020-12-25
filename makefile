$(shell mkdir -p obj bin)

comma = ,
sizeString = $(subst $(comma),_,$(size))

CC=g++-10 --std=c++20
CFLAGS=-O3 -g -W -Wall

VARIADIC_ARRAY=src/variadic-array.hpp
PERMUTATION=src/permutation.hpp src/permutation.tpp $(VARIADIC_ARRAY)
SLICE=src/slice.hpp src/slice.tpp $(PERMUTATION)

help:
	@echo "make tile size=A,B,C,... : run the optimal tiling algorithm on a given size"
	@echo "make cube size=A,B,C,... : run the optimal cube algorithm on a given size"
	@echo "make all  size=A,B,C,... : compile all programs without running"
	@echo ""
	@echo "For performance concerns, it is suggested that the dimension sizes be given\
	 in nonascending order."

all: bin/optimal_tile_$(sizeString) bin/optimal_cube_$(sizeString)

tile: bin/optimal_tile_$(sizeString)
	./bin/optimal_tile_$(sizeString)

cube: bin/optimal_cube_$(sizeString)
	./bin/optimal_cube_$(sizeString)

debug_tile: bin/optimal_tile_$(sizeString)
	gdb ./bin/optimal_tile_$(sizeString)

debug_cube: bin/optimal_cube_$(sizeString)
	gdb ./bin/optimal_cube_$(sizeString)

perf_tile: bin/optimal_tile_$(sizeString)
	perf record /bin/optimal_tile_$(sizeString)

perf_cube: bin/optimal_cube_$(sizeString)
	perf record /bin/optimal_cube_$(sizeString)

obj/optimal_tile_$(sizeString).o: src/optimal_tile.cpp $(SLICE)
obj/optimal_cube_$(sizeString).o: src/optimal_cube.cpp $(SLICE)

obj/equivRelation.o: src/equivRelation.cpp src/equivRelation.hpp

obj/%.o:
	$(CC) $(CFLAGS) -DDIM_SIZES=$(size) -c $< -o $@

bin/optimal_tile_$(sizeString): obj/optimal_tile_$(sizeString).o obj/equivRelation.o
bin/optimal_cube_$(sizeString): obj/optimal_cube_$(sizeString).o obj/equivRelation.o

bin/%:
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f obj/* bin/*
