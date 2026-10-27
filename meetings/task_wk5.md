
## WP1. Problem Definition & Modelling (Open Issue)
One modelling issue we need to resolve:
Our current problem statement specifies that:

- The path is planned before movement begins.
- At most one edge may be blocked.
- The blockage is only discovered when the traveller reaches the node.
However, we currently do not specify what happens after the traveller encounters a blockage.
Some candidate metrics (e.g. robustness, fragility, recovery cost) depend on this behaviour, so different assumptions may lead to different results.
Examples:

- Return to source.
- Backtrack to an earlier branching point.
- Reroute from current location.
- Follow a precomputed contingency plan.
#### Need opinions on whether:

- recovery behaviour should be fixed as part of the model, or
- recovery planning should itself be part of the optimisation problem.


## WP2. Existing Methods / Literature (3 slots)
#### Goal:

- Find existing methods from related problems.
- Understand their assumptions.
- Analyse whether their ideas transfer to our setting.
#### Potential areas:

- Canadian Traveller Problem
- Robust Shortest Path
- Multi-objective Routing
- Network Reliability
#### For Week 7:

- 1 well-understood method is enough.
#### For final report:

- Ideally ~3 existing approaches analysed.
### Slots:

- [ Arseniy Pyanov ]
- [ ]
- [ ]


## WP3. Our Own Formulation / Metrics (2 slots)
#### Goal:

- Build our own formulation.
- Collect counterexamples where shortest path behaves poorly.
- Identify desirable path characteristics.
- Define candidate robustness metrics.
- Develop our own algorithmic ideas.
#### Current tasks:

- Counterexample document.
- Mathematical representation of desirable properties.
- Candidate metrics / objectives.
#### For Week 7:

- At least 1 candidate metric + some analysis.
#### For final report:

- Ideally 2 formulations / approaches.
### Slots:

- [ ]
- [ ]


## WP4. Experimental Environment (5 slots)
####Goal:
Build common infrastructure so we can test algorithms later.
#### Slot 1 [ ]
Graph representation

- graph.h
- graph loading
- adjacency list structures
#### Slot 2 [ ]
Graph generation

- random graph generation
- custom test instances
- seeded generation
#### Slot 3 [ ]
Basic algorithms

- BFS
- DFS
- shortest path helpers
- SCC helpers if needed
#### Slot 4 [ ]
Visualisation

- DOT generation
- Graphviz scripts
- graph debugging visualisations
#### Slot 5 [ ]
Experiment & Analysis Pipeline

- Kaggle notebook setup
- compile / run scripts
- batch experiment execution
- result collection
- Python plotting
- comparison graphs
- result visualisation
(Currently thinking of using Kaggle with shell ! commands to compile/run everything from one notebook instead of building a large local experiment framework.)


### Please have a look and comment if:

- WP1 is missing any important modelling assumptions,
- the work distribution looks reasonable,
- you'd prefer a different slot,
- or you have other ideas for metrics / related work.
- take 1 slot form WP2/3, and 1 slot from 4
