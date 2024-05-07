# COL380_A3
Maze creation &amp; solving using MPI

# Workflow

## 1. Generating the maze

- A min-spanning tree would satisfy the condition that there is only one path between any 2 cells (nodes)
- There should thus be some underlying graph structure on top of which we want to create the min-spanning tree
- Possible ideas for the randomness of the maze:
    - Generate a random connected graph and then create the min-spanning tree on top of that.
    - Assume that each node in the graph is connected to all its immediate neighbours and each edge has same weight-> i.e. its now a deterministic graph
        - In this case we'd have to generate a random root in the graph
        - Each time we try to find a neighbour we'd have to select a random neighbour instead of deterministically traversing its edge list (since all edges are of equal weight)
    -  To simulate both option 1 and option 2: All the nodes in the graph is connected to all its immediate neighbours and each edge has a random weight. Now the graph algorithms would give a random min-spanning tree
        - Just like before we'll try to find the min spanning tree from a randomly generated root in the graph
        - Note that this idea is basically like idea 1 in that it generates a random connected graph (just that now all edges have non-zero weight) and it is like idea 2 in that all neighbours are connected
- The graph should thus be generated in mazegenerator.cpp which would then call bfs.cpp or kruskal.cpp (depending on the command line arguments) and generate the required min-spanning tree using that algo

### 1.1 Generating the min-spanning tree




# Todo

## 1. Data structures

- Nodes of the maze are represented by an integer, 64*row + col
- We make macros to access row no. and col no., neighbors, etc.
- We can store edges as either:
    - hashmapping node to 4bit value representing whether node is connected to left/down/up/right neighbors (can decide on direction ordering later)

