<!-- This README explains the experiment scaffold and the purpose of its files. -->

# CTP Experiment

This directory is the starting environment for graph experiments. Priorities
1–4 in `meetings/exp_setup.md` are implemented: the project has a BGL graph
type, seeded instance generation, blockage validation, and graph persistence.
Priorities 5, 10, 11, and 12 are implemented as the Dijkstra/Backtrack baseline,
batch runner, DOT export, and notebook analysis. The path-tree and selection
priorities remain for later work.

## File structure

```text
ctp-experiment/
├── Makefile
├── README.md
├── include/                  C++ declarations shared across modules
├── src/                      C++ module implementations and program entry point
├── tests/                    Focused tests for graph and algorithm behavior
├── config/                   Experiment input configurations
├── scripts/                  Build, run, and graph-rendering helpers
├── data/
│   ├── generated/             Generated graph instances
│   └── results/               Experiment measurements and outputs
├── visualisations/
│   ├── dot/                   Graphviz DOT source files
│   └── images/                Rendered graph images
└── notebooks/                 Notebook-based experiment analysis
```

### Build file

- `Makefile` — builds the graph-infrastructure CLI and its focused tests.

### Headers in `include/`

- `graph.hpp` — weighted undirected BGL graph, generation settings, and instance
  data types.
- `graph_generator.hpp` — seeded graph and blockage generation interface.
- `graph_utils.hpp` — connectivity checks, instance validation, and graph
  serialization interfaces.
- `baseline_algorithms.hpp` — Dijkstra and offline Backtrack plan interfaces.
- `path_tree.hpp` — declarations for the backwards path-tree representation.
- `ctp_algorithms.hpp` — interfaces for CTP route selection and recovery.
- `visualizer.hpp` — graph visualization interfaces.
- `experiment.hpp` — single-run metrics and configured batch execution.
- `logger.hpp` — CSV serialization for plans and performance results.
- `config.hpp` — experiment CSV loading and validation.

### Sources in `src/`

- `graph_generator.cpp` and `graph_utils.cpp` — implementations for seeded graph
  generation, BGL connectivity checks, validation, and graph persistence.
- `baseline_algorithms.cpp` — BGL Dijkstra and Backtrack recovery precomputation.
- `path_tree.cpp` and `ctp_algorithms.cpp` — placeholders for the path tree and
  CTP-specific behavior.
- `visualizer.cpp` — generates graph visualization output.
- `config.cpp` — reads per-instance parameters, independent seeds, and repeats.
- `experiment.cpp` and `logger.cpp` — measures runs and writes result rows.
- `main.cpp` — `generate`, `validate`, and batch `run` commands.

### Supporting files

- `tests/test_graph.cpp` — checks seeded reproducibility, blockage connectivity,
  configuration validation, and save/load round trips.
- `tests/test_baseline.cpp` — verifies shortest paths, Backtrack walk format,
  costs, and infeasible recovery reporting.
- `tests/test_experiment.cpp` and `test_visualizer.cpp` — verify batch outputs
  and DOT annotations.
- `tests/test_generator.cpp`, `test_path_tree.cpp`, `test_recovery.cpp`, and
  `test_selection.cpp` — reserved for more focused test suites.
- `config/experiment_config.csv` — graph parameters, independent graph and
  blockage seeds, and repetition count.
- `scripts/build.sh` — invokes the Makefile build.
- `scripts/run_experiments.sh` — builds and runs the configured batch.
- `scripts/render_graphs.sh` — renders DOT files with Graphviz.
- `data/` — keeps generated instances and results separate.
- `visualisations/` — separates editable DOT files from rendered images.
- `notebooks/experiments.ipynb` — Kaggle workflow to clone/update the repository,
  build and run the Bash scripts, then analyze CSV results and display rendered
  graph images. Private GitHub access uses Kaggle Secrets, not a token in code.

## Build and use

Boost Graph Library headers and a C++17 compiler are required. Build and run the
focused graph-infrastructure tests with:

```sh
make
make test
```

The build requires a C++17 compiler and Boost Graph Library headers.

Generate and then validate an instance with:

```sh
./build/ctp generate --vertices 20 --edge-probability 0.15 \
  --min-weight 1 --max-weight 10 --seed 1001 --blockage-seed 2001 \
  --output data/generated/instance.ctp
./build/ctp validate data/generated/instance.ctp
```

The generator first creates a seeded random spanning tree, then adds a seeded
random extra edge so at least one blockage candidate exists. Additional
non-tree edges are included according to `edge_probability`. It chooses distinct
source and destination vertices, then selects a blocked edge only after checking
that an s-to-t route remains. The graph file stores the parameters, seed,
blockage seed, endpoints, blocked edge, and weighted edge list in a documented
text format. If no blockage seed is provided, the graph seed is reused.

## Offline Backtrack baseline

The Backtrack baseline first selects a shortest initial route with Dijkstra.
Before travel begins, it then processes every edge on that route: it removes the
edge, computes a new Dijkstra route from `s` to `t`, and builds the corresponding
recovery walk by reversing the already-traveled prefix back to `s` before
following that new route. This models the textbook Backtrack policy while
moving all Dijkstra work into preprocessing.

`run_backtrack` returns a `std::vector<std::vector<int>>`, and
`BacktrackPlan::paths` exposes the same output alongside measurements. It follows
[`../doc/algorithm_output.md`](../doc/algorithm_output.md):
each entry starts at its initial-route vertex and ends at `t`, and the final
entry is `{t}`. The first elements reconstruct the initial route. If removing
any initial-route edge leaves no s-to-t route, the algorithm reports the
specific infeasible edge; the runner records an `infeasible` result and does not
emit an invalid path plan.

## Running experiments

The CSV requires `vertex_count`, `edge_probability`, `min_weight`, `max_weight`,
`edge_seed`, `blockage_seed`, and `repetitions`. Each repeat increments both
seeds, so every saved graph and generated blockage can be reproduced.

```sh
./scripts/run_experiments.sh
# or choose output locations directly:
./build/ctp run --config config/experiment_config.csv \
  --results data/results/results.csv \
  --graphs data/generated --dots visualisations/dot
```

Each run saves a graph instance and DOT visualization. The result CSV includes
the algorithm's nested path vector and per-scenario measurements as JSON, graph
and preprocessing times, precomputed recovery lookup time, travel costs,
worst-case competitive ratio, and estimated in-memory plan storage. Recovery
lookup measures selecting the
precomputed scenario; it does not include Dijkstra. Actual travel cost reflects
the generated blockage if it lies on the selected initial path, otherwise the
traveler reaches `t` without encountering that blockage.

Render DOT files to PNG with Graphviz:

```sh
./scripts/render_graphs.sh
```

The notebook checks for compiler, Boost, Graphviz, pandas, and matplotlib, and
installs missing runtime dependencies in Kaggle. Set `REPO_URL` in its first
configuration cell; add a `GITHUB_TOKEN` Kaggle Secret for a private GitHub
repository, or set `GITHUB_TOKEN_SECRET_NAME` to `None` for a public repository.
Infeasible runs are counted separately from successful path plans.

The generation structure is intentionally a first reproducible baseline, not a
claim that this is the only or most representative random graph model. Later
experiment stages can add graph families without changing the saved-instance
contract.
