## Expected Algorithm Output — Offline 1-CTP Planning

The algorithm must output a `std::vector<std::vector<int>>` representing the selected initial path and its precomputed recovery paths.

```cpp
std::vector<std::vector<int>> paths;
```

### 1. Output structure

Each inner vector represents a complete route from a vertex on the initial path to the destination `t`.

- `paths[i][0]` identifies the i-th vertex on the initial path.
- `paths[i]` stores the route to destination `t`, assuming the next edge of the initial path is blocked at that vertex.
- Every inner vector must end at the destination `t`.
- The initial path is reconstructed by collecting the first element of every inner vector, in order.

### 2. Example

Suppose the selected initial path is:

```text
0 → 1 → 2 → 3 → 4
```

The expected output could be:

```cpp
std::vector<std::vector<int>> paths = {
    {0, 8, 1, 2, 3, 4}, // Recovery if edge (0,1) is blocked
    {1, 5, 7, 9, 4},    // Recovery if edge (1,2) is blocked
    {2, 10, 4},         // Recovery if edge (2,3) is blocked
    {3, 2, 10, 4},      // Recovery if edge (3,4) is blocked
    {4}                  // Destination / terminal entry
};
```

The initial path is reconstructed as:

```cpp
0 -> 1 -> 2 -> 3 -> 4
```

### 3. Important requirements

1. **Initial path:** The first element of each inner vector must collectively represent one valid initial path from source `s` to destination `t`.

2. **Recovery paths:** Each `paths[i]` must represent a valid route starting at `paths[i][0]` and ending at `t`, avoiding the corresponding blocked edge.

3. **Recovery may move backwards:** A recovery path does not have to continue forward along the original path. It may revisit earlier vertices on the initial path or use other parts of the graph.

4. **No blockage:** The initial path is the route followed when no blockage is encountered. It is reconstructed from the first elements of the inner vectors.

5. **Terminal entry:** The final entry is `{t}` because there is no next edge to block after reaching the destination.

6. **At most one blockage:** Each recovery path handles one possible blockage on the initial path. The algorithm does not need to handle multiple simultaneous blockages.

7. **No invalid routes:** If a valid recovery route does not exist for a blockage scenario, the algorithm must report that scenario as infeasible rather than silently returning an invalid path.

### 4. What the algorithm is expected to achieve

The algorithm should:

- Select an initial source-to-destination path.
- Precompute a recovery path for every edge along that initial path.
- Store all paths in the specified nested-vector format.
- Allow the traveller to identify the appropriate recovery path immediately after discovering a blockage.

The algorithm's output is an offline plan. It should not require running a new pathfinding algorithm during execution to determine the recovery route.

**Note:** This output format specifies the required result, not the path-selection objective. The algorithm used to select the initial path and recovery paths can be developed separately.