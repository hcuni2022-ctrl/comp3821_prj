# Proposed Algorithm Architecture — 1-CTP

## 1. Objective

The proposed approach uses two data structures to organise and retrieve possible routes in an undirected weighted graph:

1. **Path-enumeration tree:** Convert the original graph into a tree representation that enumerates all possible simple paths towards the destination.
2. **Directed path index:** Convert the tree's path information into an indexed, directed graph-like structure that groups candidate paths by their next vertex.

The purpose is to precompute alternative routes so that, when a blockage is discovered, the traveller can quickly retrieve a suitable recovery path without running a new pathfinding algorithm.

**Important:** We have NOT yet designed the method for selecting the initial path. The current design focuses on generating, organising and retrieving candidate paths. Initial-path selection is a separate, unresolved part of the project.

---

## 2. Stage One — Original Undirected Graph to Path-Enumeration Tree

### Input

An undirected weighted graph:

`G = (V, E, w)`

Where:
- `V` represents vertices.
- `E` represents undirected edges.
- `w` represents edge weights.
- `s` is the source.
- `t` is the destination.

### Step 1: Root the tree at the destination

Start building the tree from destination `t`, expanding backwards through the original graph.

**Reason:**
- All candidate paths should eventually lead to the same destination.
- Building backwards allows each tree node to store its accumulated cost to `t`.
- It makes it convenient to reconstruct routes by following parent pointers towards the root.

### Step 2: Create path-specific tree nodes

Each tree node represents an instance of an original graph vertex along a particular path.

The same original vertex may appear multiple times in the tree if it belongs to different paths.

For example, original vertex `2` may appear in several tree nodes because it can be reached through different path histories.

Each tree node should contain:

| Field | Purpose |
|---|---|
| `vertexID` | ID of the corresponding original graph vertex |
| `accumulatedCost` | Total cost from this node to destination `t` |
| `parent` | Next tree node towards destination `t` |
| `pathHistory` | Vertices or edges already visited along this path |
| `children` | Possible preceding vertices from the original graph |

**Reason:**

A normal graph vertex does not preserve the path used to reach it. Separate tree-node instances allow us to distinguish different routes even when they contain the same original vertex.

### Step 3: Enumerate all simple paths

Expand the tree while preventing cycles.

When expanding a node, do not visit an original graph vertex that already appears in that node's path history.

For example:

- Valid: `0 -> 1 -> 2 -> 4`
- Invalid: `0 -> 1 -> 2 -> 1 -> 4`

The objective is to enumerate all simple paths rather than arbitrary walks containing cycles.

### Step 4: Maintain a mapping from original vertices to tree nodes

Maintain a separate mapping:

```cpp
std::unordered_map<int, std::vector<TreeNode*>> vertexToTreeNodes;
```

- Key: original graph vertex ID.
- Value: all tree-node instances representing that vertex.

**Reason:**

The same original vertex may appear in multiple tree locations. This mapping allows us to retrieve all its path-specific instances without traversing the entire tree.

### Stage One output

A rooted path-enumeration tree containing:
- All enumerated simple paths.
- Separate tree-node instances for different path histories.
- Accumulated costs to destination.
- Parent/child relationships for path reconstruction.
- A mapping between original vertices and their tree-node instances.

---

## 3. Stage Two — Tree Information to Directed Path Index

The path-enumeration tree is useful for generating and preserving candidate paths. However, searching through the tree repeatedly to find alternative routes may be inconvenient.

We therefore propose constructing a second structure: a directed graph-like path index.

### Step 5: Identify possible next vertices

For each original vertex `u`, identify the possible next vertices `v` that appear immediately after `u` in candidate paths towards destination `t`.

Represent these as directed transitions:

```text
u -> v
```

Although the original graph is undirected, these transitions are directed because they represent movement along candidate routes towards the destination.

Both directions of an original edge may appear if different candidate paths use that edge in opposite directions.

### Step 6: Group candidate paths by their next vertex

The proposed index is:

```cpp
std::unordered_map<
    int,
    std::unordered_map<int, std::vector<Path>>
> pathIndex;
```

Conceptually:

```cpp
pathIndex[currentVertex][nextVertex]
```

Where:
- First key: current vertex.
- Second key: possible next vertex.
- Value: all candidate paths starting at `currentVertex` whose next vertex is `nextVertex`.

For example, suppose the candidate paths from vertex `2` are:

```text
2 -> 3 -> 4
2 -> 3 -> 5 -> 4
2 -> 10 -> 4
2 -> 10 -> 7 -> 4
```

The index would contain:

```cpp
pathIndex[2][3] = {
    {2, 3, 4},
    {2, 3, 5, 4}
};

pathIndex[2][10] = {
    {2, 10, 4},
    {2, 10, 7, 4}
};
```

**Reason:**
- Candidate paths sharing the same next vertex are grouped together.
- A blocked edge can be identified through its corresponding next-vertex group.
- We can reject an entire group instead of checking every candidate path individually.
- Candidate paths can be sorted by accumulated cost to help retrieve cheaper alternatives first.

The current vertex must also be part of the key. Using only `nextVertex` would be ambiguous because the same vertex can be reached from multiple current vertices.

### Stage Two output

A directed path index that allows candidate paths to be retrieved using:

```cpp
pathIndex[currentVertex][nextVertex]
```

This structure is intended to make alternative-path lookup and filtering more convenient.

---

## 4. Stage Three — Recovery Path Retrieval

The directed index is used to retrieve recovery paths when a blockage is discovered.

### Example

Suppose the traveller is currently at vertex `2` and discovers that edge `(2,3)` is blocked.

Retrieve:

```cpp
pathIndex[2]
```

The candidate paths are grouped as follows:

```text
Next vertex 3:
    2 -> 3 -> 4
    2 -> 3 -> 5 -> 4

Next vertex 10:
    2 -> 10 -> 4
    2 -> 10 -> 7 -> 4
```

Since edge `(2,3)` is blocked, reject the group associated with next vertex `3`.

The remaining candidates are:

```text
2 -> 10 -> 4
2 -> 10 -> 7 -> 4
```

Select a valid recovery path from the remaining candidates.

For the initial implementation, candidate paths may be sorted by total remaining path cost, allowing the cheapest valid candidate to be retrieved first.

Note that recovery paths may move backwards along the original initial path. They do not have to follow the original path's direction of progress.

---

## 5. Expected Final Algorithm Output

The final offline plan should use:

```cpp
std::vector<std::vector<int>> paths;
```

Each inner vector represents a complete route from a vertex on the selected initial path to destination `t`.

The first element of each inner vector identifies the corresponding vertex on the initial path.

For example:

```cpp
std::vector<std::vector<int>> paths = {
    {0, 8, 1, 2, 3, 4}, // Recovery if edge (0,1) is blocked
    {1, 5, 7, 9, 4},    // Recovery if edge (1,2) is blocked
    {2, 10, 4},         // Recovery if edge (2,3) is blocked
    {3, 2, 10, 4},      // Recovery if edge (3,4) is blocked
    {4}                  // Destination / terminal entry
};
```

The initial path is reconstructed by collecting the first element of every inner vector:

```text
0 -> 1 -> 2 -> 3 -> 4
```

Output conventions:

- `paths[i][0]` identifies the i-th vertex on the initial path.
- `paths[i]` stores the route from that vertex to `t`, assuming the next edge of the initial path is blocked.
- Every route must end at `t`.
- The final `{t}` entry represents the destination and is not a blockage scenario.
- Each recovery route must avoid its corresponding blocked edge.
- If a valid recovery route does not exist, the scenario must be reported as infeasible.
- The plan handles at most one blockage.

---

## 6. Overall Architecture

```text
Original Undirected Weighted Graph
                |
                v
     Path-Enumeration Tree
                |
                | Enumerate simple paths
                | Store path histories and costs
                | Map original vertices to tree nodes
                |
                v
       Directed Path Index
                |
                | Group paths by current vertex
                | and possible next vertex
                |
                v
      Recovery Path Retrieval
                |
                v
       Final Offline Plan
      vector<vector<int>>
```

## 7. Current Scope and Unresolved Questions

### What we have proposed

- Transforming the original undirected graph into a path-enumeration tree.
- Enumerating simple paths while preventing cycles.
- Maintaining a mapping from original vertices to tree-node instances.
- Building a directed path index that groups paths by current vertex and next vertex.
- Using the index to filter out paths containing a blocked outgoing edge.
- Producing a collection of precomputed recovery paths.

### What we have NOT designed yet

**Initial-path selection is still unresolved.**

We have not decided how to choose the initial source-to-destination path from all the candidate paths.

In particular, we have not established:
- The selection objective or cost function.
- Whether to minimise worst-case total travel cost, competitive ratio, or another metric.
- How the quality and availability of recovery paths should influence the initial-path choice.
- Whether the proposed representation can support an efficient selection method.

Do not assume that the shortest path is automatically the selected initial path. Do not implement a final initial-path selection strategy without further discussion.

### Important implementation considerations

1. **Exponential growth:** The number of simple paths can grow exponentially with graph size. The tree and index may consume substantial time and memory.

2. **Avoid unnecessary duplication:** The initial prototype may store complete paths in vectors. Later, consider storing path IDs or references to tree nodes to avoid copying the same route repeatedly.

3. **Separate preprocessing and lookup:** Measure tree construction, index construction, recovery-path retrieval and memory usage separately.

4. **Validate recovery paths:** Every recovery path must start at the correct current vertex, reach `t`, and avoid the corresponding blocked edge.

5. **Do not claim efficiency improvements yet:** The directed index is intended to improve organisation and lookup. Whether it is actually faster than alternatives such as repeated Dijkstra must be established experimentally.
<!-- 
**Instruction to coding agent:** Implement the graph-to-tree transformation and the directed path index as separate modules. Keep the initial-path selection logic independent and unimplemented until its objective has been agreed upon. -->