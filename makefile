$(shell mkdir -p obj bin)

CC=g++-10 --std=c++20
CFLAGS= -O3 -g -W -Wall

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

all: bin/optimal_tile_$(size) bin/optimal_cube_$(size)

tile: bin/optimal_tile_$(size)
	./bin/optimal_tile_$(size)

cube: bin/optimal_cube_$(size)
	./bin/optimal_cube_$(size)

debug_tile: bin/optimal_tile_$(size)
	gdb ./bin/optimal_tile_$(size)

debug_cube: bin/optimal_cube_$(size)
	gdb ./bin/optimal_cube_$(size)

perf_tile: bin/optimal_tile_$(size)
	perf record /bin/optimal_tile_$(size)

perf_cube: bin/optimal_cube_$(size)
	perf record /bin/optimal_cube_$(size)

obj/optimal_tile_$(size).o: src/optimal_tile.cpp $(SLICE)
obj/optimal_cube_$(size).o: src/optimal_cube.cpp $(SLICE)

obj/equivRelation.o: src/equivRelation.cpp src/equivRelation.hpp

obj/%.o:
	$(CC) $(CFLAGS) -DDIM_SIZES=$(size) -c $< -o $@

bin/optimal_tile_$(size): obj/optimal_tile_$(size).o obj/equivRelation.o
bin/optimal_cube_$(size): obj/optimal_cube_$(size).o obj/equivRelation.o

bin/%:
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f obj/* bin/*
