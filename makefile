$(shell mkdir -p obj bin)

ifdef trace
	trace_flag=-t
else
	trace_flag=
endif

CFLAGS= --std=c++2a -O3 -g -W -Wall

CC=g++-10

help:
	@echo "make all, compile all programs"
	@echo ""
	@echo "make tile size=N, run the optimal tiling algorithm on a given width"
	@echo "make rect size=N, run the optimal rectangle algorithm on a given width"
	@echo "make rect_matrix, run the optimal rectangle matrix algorithm"
	@echo "make cube size=N, run the optimal cube tiling algorithm with a given square base size"
	@echo ""
	@echo "To add tracing to any program, give the variable 'trace' any value. For example:"
	@echo "make tile size=5 trace=y"

all: bin/optimal_tiling bin/optimal_rectangle bin/optimal_rectangle_matrix

tile: bin/optimal_tiling
	./bin/optimal_tiling $(size) $(trace_flag)

rect: bin/optimal_rectangle
	./bin/optimal_rectangle $(size) $(trace_flag)

rect_matrix: bin/optimal_rectangle_matrix
	./bin/optimal_rectangle_matrix $(trace_flag)

cube: bin/optimal_cube
	./bin/optimal_cube $(size) $(trace_flag)

#debug: bin/optimal_tiling
#	gdb --args ./bin/optimal_tiling $(size) -t

#perf_run: $(OT_efile)
#	perf record ./$(TE_efile) $(size)

obj/%.o: src/%.cpp src/%.hpp
	$(CC) $(CFLAGS) -c $< -o $@

# Empty target for .cpp files that do not have a .hpp counterpart.
%.hpp: ;

bin/%: obj/%.o obj/equivRelation.o obj/fraction.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f obj/* bin/*
