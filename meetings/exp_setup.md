# Project Context: COMP3821 — Fragility-Aware 1-Canadian Traveller Problem (1-CTP)

## 1. Project Overview

I am developing a theoretical and experimental investigation into a variation of the Canadian Traveller Problem (CTP), specifically the **1-Canadian Traveller Problem (1-CTP)**.

The project investigates whether graph structure and precomputed alternative-path information can improve initial path selection when an edge may become blocked during traversal.

The central motivation is:

> A shortest path is not necessarily the best initial path when a blockage can force the traveller to backtrack or take an expensive alternative route.

The project is exploratory. Do not assume that the proposed approach is novel, optimal or more efficient than existing algorithms. These are questions to investigate experimentally.

## 2. Problem Definition

The environment is represented by a weighted, undirected graph:

\[
G=(V,E,w)
\]

Where:
- \(V\): vertices.
- \(E\): undirected edges.
- \(w(e)>0\): positive edge weights.
- \(s\): source vertex.
- \(t\): destination vertex.

The traveller knows the graph structure and edge weights before departure.

However, the traveller does not know which edge is blocked. A blockage is discovered only when the traveller reaches an endpoint and attempts to traverse that edge.

### Project assumptions

- Undirected graph.
- Positive edge weights.
- At most one blocked edge (\(k=1\)).
- A blockage is permanent.
- The blockage is discovered at an endpoint of the blocked edge.
- The graph remains sufficiently connected that an alternative \(s\)-to-\(t\) route exists after the blockage.
- No probabilistic blockage predictions.
- No adversarial or game-theoretic blockage placement.

The project focuses on initial path selection and recovery planning, rather than repeatedly running a shortest-path algorithm after every blockage.

## 3. Main Research Motivation

Consider two initial routes:

- Route A: one edge with weight 65.
- Route B: two edges with weights 59 and 1, giving a total of 60.

Although Route B is shorter, if its final edge is blocked, the traveller may need to retrace the expensive 59-weight edge.

Therefore, minimising initial path length alone may not minimise total travel cost.

The project investigates how path structure, alternative routes and recovery costs should influence initial path selection.

## 4. Current Algorithm Direction

The current proposed approach uses a **backwards path-enumeration tree rooted at destination \(t\)**.

### Path tree construction

- Begin at destination \(t\).
- Expand backwards through the original graph.
- Each tree node represents one instance of an original graph vertex along a particular simple path towards \(t\).
- The same original vertex may appear in multiple tree nodes because it can belong to multiple distinct paths.
- Cycles are prohibited within each represented path.
- Each tree node stores its original vertex ID and accumulated path cost towards \(t\).
- Parent/child relationships allow path reconstruction.

### Linking the tree to the original graph

Maintain a mapping from each original graph vertex to all corresponding tree-node instances.

For example:

```cpp
unordered_map<int, vector<TreeNode*>> vertexToTreeNodes;
```

Each original vertex can then access its precomputed candidate continuations towards \(t\).

Candidate continuations can be sorted by accumulated path cost.

### Recovery lookup

When the traveller reaches vertex \(u\) and discovers that edge \(e=(u,v)\) is blocked:

1. Retrieve the candidate continuation paths associated with \(u\).
2. Consider candidates in ascending accumulated cost.
3. Reject candidates containing the blocked edge.
4. Select the cheapest remaining valid continuation.

Conceptually:

\[
Q^* =
\arg\min_{\substack{Q\in\mathcal P(u,t)\\e\notin Q}}
w(Q)
\]

Where \(\mathcal P(u,t)\) represents the set of simple paths from \(u\) to \(t\).

This is an offline precomputation approach: candidate paths and their costs are calculated before traversal, rather than running Dijkstra again when a blockage is discovered.

**Important:** this is the current proposed architecture, not a proven improvement. Its computational and memory costs must be evaluated, particularly because the number of simple paths can grow exponentially.

## 5. Project Objectives

The implementation should eventually support:

1. Generating reproducible weighted undirected graphs.
2. Generating one blockage while preserving an available \(s\)-to-\(t\) route.
3. Computing baseline shortest paths using Dijkstra.
4. Evaluating Backtrack and recovery costs.
5. Constructing the backwards path-enumeration tree.
6. Linking tree nodes to original graph vertices.
7. Filtering candidate recovery paths against a blocked edge.
8. Investigating methods for selecting a better initial route.
9. Comparing the proposed approach against baseline methods.
10. Measuring preprocessing time, memory consumption, recovery lookup time and travel costs.

## 6. Experimental Environment

### Programming language and libraries

- C++17 or later.
- Boost Graph Library (BGL) for graph representation and standard graph algorithms.
- Graphviz for graph and path visualisation.
- Bash for build and experiment automation.
- Python with pandas and matplotlib for experimental analysis.
- Kaggle notebooks for larger experiment batches, after local testing.

### Intended project structure

```text
ctp-experiment/
│
├── include/
│   ├── graph.hpp
│   ├── graph_generator.hpp
│   ├── graph_utils.hpp
│   ├── baseline_algorithms.hpp
│   ├── path_tree.hpp
│   ├── ctp_algorithms.hpp
│   ├── visualizer.hpp
│   ├── experiment.hpp
│   ├── logger.hpp
│   └── config.hpp
│
├── src/
│   ├── graph_generator.cpp
│   ├── graph_utils.cpp
│   ├── baseline_algorithms.cpp
│   ├── path_tree.cpp
│   ├── ctp_algorithms.cpp
│   ├── visualizer.cpp
│   ├── experiment.cpp
│   ├── logger.cpp
│   └── main.cpp
│
├── tests/
│   ├── test_graph.cpp
│   ├── test_generator.cpp
│   ├── test_path_tree.cpp
│   ├── test_recovery.cpp
│   └── test_selection.cpp
│
├── config/
├── scripts/
│   ├── build.sh
│   ├── run_experiments.sh
│   └── render_graphs.sh
│
├── data/
│   ├── generated/
│   └── results/
│
├── visualisations/
│   ├── dot/
│   └── images/
│
├── notebooks/
│   └── experiments.ipynb
│
├── Makefile
└── README.md
```

## 7. Baseline Algorithms

Use BGL implementations where available rather than unnecessarily reimplementing standard algorithms.

Relevant BGL functionality includes:

- Dijkstra shortest paths.
- Breadth-first search.
- Depth-first search.
- Connected components.
- Graphviz DOT output.

Write project-specific wrappers where a consistent interface or additional CTP functionality is needed.

Baseline comparisons should include:

- Dijkstra shortest initial path.
- Backtrack recovery strategy.
- Proposed precomputed path-tree recovery approach.
- Proposed initial-path selection method, once implemented.

## 8. Experimental Measurements

Record at least:

| Metric | Purpose |
|---|---|
| Number of vertices | Graph size |
| Number of edges | Graph density |
| Random seed | Reproducibility |
| Blocked edge | Blockage scenario |
| Initial path cost | Initial route quality |
| Preprocessing time | Cost of constructing path information |
| Memory consumption | Scalability |
| Recovery lookup time | Cost of finding a continuation |
| Recovery cost | Cost after blockage |
| Worst-case total travel cost | Overall strategy performance |
| Competitive ratio | Comparison against offline optimum |

Run multiple seeds and graph instances for each configuration. Do not draw conclusions from a single randomly generated graph.

Keep preprocessing time separate from online recovery lookup time.

## 9. Implementation Priorities

Develop the project incrementally in this order:

1. BGL graph representation.
2. Reproducible graph generation using random seeds.
3. Blockage generation and \(s\)-to-\(t\) connectivity validation.
4. Graph saving and loading.
5. Dijkstra wrapper and baseline recovery evaluation.
6. Backwards path-tree construction.
7. Vertex-to-tree-node mapping.
8. Candidate recovery-path filtering.
9. Initial-path selection mechanism.
10. Experiment runner, timing and logging.
11. DOT visualisation and Graphviz rendering.
12. Python/Kaggle experimental analysis.

## 10. Instructions for the Coding Agent

- Implement incrementally; do not generate the entire project in one step.
- Prioritise correctness and testability over premature optimisation.
- Reuse BGL functionality where appropriate.
- Keep graph generation, graph operations, baseline algorithms, proposed algorithms, visualisation and experiment management separate.
- Use reproducible random seeds.
- Avoid hardcoding graph sizes, seeds and experiment parameters.
- Add unit tests using small, manually constructed graphs with known expected results.
- Clearly distinguish preprocessing from recovery lookup.
- Do not assume the path-tree approach is efficient: measure its time and memory consumption.
- Do not invent a final initial-path selection metric. The selection mechanism is still under investigation.
- Explain important design decisions before making major architectural changes.

**Immediate implementation task:** Build the graph infrastructure first. Generate a weighted undirected BGL graph, choose source and destination, generate a single blocked edge, verify that an alternative source-to-destination route exists after removing it, and save the reproducible graph instance.

Do not start implementing the path tree until this first milestone works reliably.