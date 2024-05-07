CXX = mpic++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -Wno-cast-function-type
INCLUDES = -I. -I./src/generator -I./src/solver -I./src

# Mention the files involved
MAZE_SRC = ./src/maze.cpp
GENERATOR_KRUSKAL = ./src/generator/kruskal.cpp
GENERATOR_BFS = ./src/generator/bfs.cpp
GENERATOR = ./src/generator/mazegenerator.cpp
SOLVER_DFS = ./src/solver/dfs.cpp
SOLVER_DIJKSTRA = ./src/solver/dijkstra.cpp
SOLVER = ./src/solver/mazesolver.cpp

SRC = $(MAZE_SRC) $(GENERATOR_KRUSKAL) $(GENERATOR_BFS) $(GENERATOR) $(SOLVER_DFS) $(SOLVER_DIJKSTRA) $(SOLVER)

# Output file
OUT = maze.out

compile: $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) -o $(OUT)

# Run kruskal generator and dijkstra solver
run_k_d: compile
	mpirun -np 4 ./$(OUT) -g kruskal -s dijkstra

# Run kruskal generator and dfs solver
run_k_f: compile
	mpirun -np 4 ./$(OUT) -g kruskal -s dfs

# Run bfs generator and dijkstra solver
run_b_d: compile
	mpirun -np 4 ./$(OUT) -g bfs -s dijkstra

# Run bfs generator and dfs solver
run_b_f: compile
	mpirun -np 4 ./$(OUT) -g bfs -s dfs

# Run all cos why not
all: compile run_k_d run_k_f run_b_d run_b_f

# Clean the output file
clean:
	rm -f $(OUT)