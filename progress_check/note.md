https://chatgpt.com/s/t_6ae9715e315881919cfa26d7302f8ca2
# COMP3821 — Algorithm Development Notes

## 1. Overall direction: From vertex-based routing to graph-structure-aware routing

- **Conventional shortest-path approach (e.g. Dijkstra):**
  - Starts from a source vertex and explores the graph to find a minimum-cost path to the destination.
  - Primarily produces one shortest path.
  - Optimises path length, but does not directly optimise how well the route can handle blocked edges or how useful alternative routes are.

- **Proposed direction:**
  - Shift the focus from selecting one path to analysing the graph structure and multiple possible paths.
  - Consider how different paths relate to one another, where they overlap, and where alternative routes can bypass a blocked edge.
  - Evaluate not only the initial path length, but also the availability and quality of alternatives if an edge becomes unavailable.
  - The goal is to find a route or routing plan that balances travel cost with the ability to recover from blockages.

- **Important distinction:**
  - This does not mean abandoning vertex-based algorithms or shortest-path algorithms.
  - Dijkstra can remain a baseline or a subroutine.
  - The proposed idea is to use information beyond a single shortest-path result when making the routing decision.

## 2. Possible approaches being investigated

These are related investigation directions, not necessarily three separate final algorithms. They could potentially be combined.

### A. Tree-based path enumeration (current data-structure idea)

- Construct a path-enumeration tree using BFS.
- Each tree node represents an instance of an original graph vertex along a particular path prefix.
- The same original vertex can appear multiple times in the tree if it is reached through different paths.
- Each node could store:
  - Parent reference.
  - Path history / predecessor information.
  - Accumulated path length.
  - References to child nodes.
  - Additional parameters or metrics used to evaluate the path.
- Maintain a hash table mapping each original graph vertex to all its corresponding tree-node instances.
- Tree nodes representing the destination correspond to different simple source-to-destination paths.
- These paths can then be evaluated and compared.

**Potential advantages**
- Provides a systematic way to generate and represent multiple simple paths.
- Allows path-specific information to be stored.
- Makes it possible to compare paths that reach the same original vertex through different histories.

**Limitations / open questions**
- The number of simple paths can grow exponentially, making full enumeration expensive.
- Simply assigning scores to tree nodes and sorting the paths may overlook relationships between different paths.
- The enumeration tree does not inherently preserve the original graph's cycles and cross-connections.
- Need to determine how to link tree nodes back to the original graph, or whether useful structural information can be embedded in the nodes.

### B. Simple-path enumeration

- Investigate generating multiple or all simple source-to-destination paths.
- A simple path does not repeat a vertex.
- This provides a candidate set of paths instead of relying only on the shortest path.
- Candidate paths could be evaluated using:
  - Total path length.
  - Potential backtracking distance.
  - Availability of alternative routes.
  - Ability to bypass blocked edges.

**Limitations / open questions**
- The number of simple paths may be exponential.
- Generating every possible path may be impractical for larger graphs.
- A list of paths alone does not necessarily capture how they connect to one another or share alternative sections.
- Need to investigate pruning or selecting a smaller set of useful candidate paths.

### C. Graph-wide structural analysis

- Instead of examining only individual paths, analyse the original graph as a whole.
- Investigate graph structures that indicate whether alternative routes exist and how resilient they are.
- Potential concepts:
  - **Cycles:** Can provide alternative connections that bypass a blocked edge.
  - **Bridges:** Edges whose removal disconnects part of the graph.
  - **Articulation vertices:** Vertices whose removal disconnects parts of the graph.
  - **Biconnected components:** Regions with stronger connectivity and alternative internal routes.
  - **Dominators / separating vertices:** Vertices that every source-to-destination path must pass through.
  - **Replacement paths:** Alternative paths that remain available when an edge or vertex on a route fails.

**Limitations / open questions**
- Not every cycle provides a useful alternative route.
- An alternative's location, length and ability to make progress towards the destination matter.
- Structural properties alone may not identify the best route; they may need to be combined with path costs and failure-related metrics.

## 3. What makes an alternative route useful?

Simply counting alternative paths or cycles may not be sufficient. Their quality and location matter.

- **Forwarding capability:**
  - Can the traveller continue towards the destination after encountering a blockage without fully retracing the route?

- **Fallback capability:**
  - If forward progress is impossible, how much travel is required to reach another viable route?

- **Alternative route quality:**
  - How much additional distance does the alternative introduce?
  - Does it avoid the blocked edge?
  - Does it lead to another vulnerable section?

- **Location of alternatives:**
  - An alternative near the beginning of a route may not help if a blockage occurs near the destination.
  - An alternative that connects two vertices around a vulnerable section may allow that section to be bypassed.

These properties may conflict with shortest-path length, so the project needs to decide how they should be evaluated or prioritised.

## 4. Current algorithmic challenges

- Define a measurable fragility or recovery-cost metric.
- Determine how to balance shortest-path length against alternative-route quality.
- Decide whether to enumerate paths, analyse graph structure, or combine both.
- Investigate whether the path-enumeration tree can retain enough information about the original graph's structure.
- Consider pruning strategies to avoid exponential enumeration.
- Compare the proposed approach against Dijkstra or other relevant routing strategies.

## 5. Suggested next steps

- Prototype the path-enumeration tree on small graphs.
- Test graphs containing cycles, bridges and multiple alternative routes.
- Compare routes with similar lengths but different recovery costs.
- Investigate dominators, biconnected components and replacement paths.
- Develop and test candidate fragility metrics.
- Determine which graph information is necessary before committing to a final algorithm.

## 6. Possible placement in the report

- **Problem motivation / background:** Explain why shortest path alone may be insufficient when edges can become unavailable.
- **Proposed approach:** Introduce the shift towards graph-structure-aware routing and evaluating multiple alternatives.
- **Algorithm design:** Describe the path-enumeration tree and any graph-wide structural analysis that is actually implemented.
- **Complexity analysis:** Discuss the cost of path enumeration and any pruning or structural algorithms.
- **Evaluation:** Compare path length, detour/backtracking cost and performance under edge blockages.
- **Limitations / future work:** Discuss exponential path growth, metric selection and the trade-off between route length and robustness.