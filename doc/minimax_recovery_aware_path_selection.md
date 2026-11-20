# Minimax Recovery-Aware Path Selection

## 1. Problem and assumptions

Let (G=(V,E,w)) be a connected, undirected graph with positive edge
weights, source (s), and destination (t).

The traveller knows the graph and all edge weights, but does not know
which edge, if any, is blocked. A blockage is discovered only when the
traveller attempts to traverse that edge. There can be **at most one
blocked edge**. Once a blockage is discovered, the traveller can use a
precomputed recovery route; no further blockage can occur.

The objective considered here is to choose a route-following strategy
that minimises the **worst-case total travel cost** from a vertex to
(t).

The algorithm uses:

1.  A path-enumeration tree rooted at (t), containing simple paths to
    (t).
2.  A directed path index grouping paths by their current vertex and
    first next vertex.
3.  A reverse-Dijkstra-style minimax calculation to choose the next
    vertex at each vertex.

The algorithm is not ordinary Dijkstra: its relaxation includes the cost
of the recovery route if the selected edge is blocked.

------------------------------------------------------------------------

## 2. Plain-English algorithm

### Stage A --- Build the path index

Enumerate simple paths from every vertex to (t). The path-enumeration
tree keeps separate tree nodes for different path histories, even when
those nodes represent the same original graph vertex.

For each original vertex `u`, group its candidate paths according to
their first next vertex `v`:

``` text
pathIndex[u][v] = candidate paths from u to t whose first edge is (u,v)
```

For the minimax calculation, only the cheapest path in each `(u, v)`
group is needed. Store its cost and enough information to reconstruct
the path.

Why is one cheapest path per group sufficient? If edge `(u,v)` is
blocked, every path in that group is invalid. All paths in other groups
start with a different edge and are simple, so they cannot later return
to `u` and use `(u,v)`. Therefore, the cheapest path in each remaining
group is sufficient to find the cheapest recovery route.

### Stage B --- Calculate recovery costs

For every directed edge choice `(u,v)`, define:

``` text
recoveryCost[u][v] =
    cheapest path from u to t that does not use edge (u,v)
```

Use the path index to calculate this value by excluding the group whose
first next vertex is `v`, then taking the cheapest path from all other
groups.

This is the cost incurred if the traveller is at `u`, attempts `(u,v)`,
discovers that it is blocked, and follows the best available recovery
route.

If no such path exists, the recovery cost is infinity and choosing
`(u,v)` is not a safe choice under the stated objective.

### Stage C --- Run a reverse-Dijkstra-style minimax algorithm

Let `dist[u]` mean:

> The minimum worst-case additional travel cost from `u` to `t`,
> assuming at most one blockage can occur from this point onward and the
> traveller follows the strategy being calculated.

At the destination:

``` text
dist[t] = 0
```

All other distances start at infinity.

Suppose the traveller at `u` chooses neighbour `v` as the next vertex.

There are two possible outcomes:

1.  **The edge works.** The traveller pays `w(u,v)` and then follows the
    strategy from `v`. The worst-case cost for this outcome is:

    ``` text
    w(u,v) + dist[v]
    ```

2.  **The edge is blocked.** The traveller follows the precomputed
    recovery route. Its cost is:

    ``` text
    recoveryCost[u][v]
    ```

The worst case for choosing `v` is the larger of these two costs:

\[
Q(u,v)=`\max`{=tex}`\left`{=tex}(w(u,v)+dist\[v\], recoveryCost\[u\]\[v\]`\right`{=tex}).
\]

The best next vertex is the one that minimises this worst-case cost:

\[ `\boxed{
dist[u]=\min_{v\in N(u)}
\max\left(w(u,v)+dist[v],\ recoveryCost[u][v]\right)
}`{=tex} \]

This is a minimax recurrence: take the maximum over the possible
outcomes of one choice, then take the minimum over the available
choices.

To calculate the values, process vertices outwards from `t` using a
min-priority queue. When a vertex `v` is settled, examine each neighbour
`u` and calculate the candidate value for choosing `v` from `u`:

``` text
candidate = max(w(u,v) + dist[v], recoveryCost[u][v])
```

If this is smaller than the current `dist[u]`, update `dist[u]` and
record `v` as `next[u]`.

Because edge weights are positive,

\[ Q(u,v)`\geq `{=tex}w(u,v)+dist\[v\]\>dist\[v\]. \]

Thus, a candidate value propagated from `v` to `u` is strictly greater
than the settled value at `v`, which gives the label-setting property
needed by the reverse-Dijkstra-style algorithm.

### Stage D --- Reconstruct the strategy

Starting at `s`, repeatedly follow `next[u]` until reaching `t`. This
gives the initial route if no blockage is encountered.

For every edge `(u,v)` on that initial route, also store the recovery
route represented by `recoveryCost[u][v]` and its corresponding
path-index entry.

The resulting strategy is:

-   If `(u,v)` works, move to `v` and continue following `next`.
-   If `(u,v)` is blocked, follow the stored recovery route from `u` to
    `t`.

The `next` pointers define the no-blockage route. The recovery paths
define what to do if an edge on that route is blocked.

------------------------------------------------------------------------

## 3. Pseudocode

The pseudocode assumes that the path index has already been built and
that each group can provide its cheapest candidate path and reconstruct
that path.

### 3.1 Build recovery costs

``` text
FUNCTION BUILD-RECOVERY-COSTS(G, pathIndex):

    FOR each vertex u:
        FOR each neighbour v of u:

            bestCost = INFINITY
            bestPath = NONE

            FOR each nextVertex x represented in pathIndex[u]:

                IF x == v:
                    CONTINUE

                candidate = cheapest path in pathIndex[u][x]

                IF candidate.cost < bestCost:
                    bestCost = candidate.cost
                    bestPath = candidate.path

            recoveryCost[u][v] = bestCost
            recoveryPath[u][v] = bestPath

    RETURN recoveryCost, recoveryPath
```

This direct version scans the available next-vertex groups for each
blocked edge. It is easy to understand and is a useful reference
implementation.

**Optional optimisation:** For each `u`, retain the cheapest and
second-cheapest groups, with different first next vertices. Then the
cheapest group excluding `v` can be obtained in constant time:

-   If the cheapest group starts with a vertex other than `v`, use it.
-   Otherwise, use the second-cheapest group.

This reduces the cost of looking up `recoveryCost[u][v]` after the group
minima have been calculated.

### 3.2 Modified reverse Dijkstra

``` text
FUNCTION MINIMAX-RECOVERY-DIJKSTRA(G, t, recoveryCost):

    FOR each vertex u:
        dist[u] = INFINITY
        next[u] = NONE

    dist[t] = 0

    priorityQueue = min-priority queue
    priorityQueue.push((0, t))

    WHILE priorityQueue is not empty:

        (cost, v) = priorityQueue.popMin()

        IF cost != dist[v]:
            CONTINUE

        FOR each neighbour u of v:

            IF recoveryCost[u][v] == INFINITY:
                CONTINUE

            successfulCost = weight(u, v) + dist[v]
            blockedCost = recoveryCost[u][v]

            candidate = MAX(successfulCost, blockedCost)

            IF candidate < dist[u]:
                dist[u] = candidate
                next[u] = v
                priorityQueue.push((candidate, u))

    RETURN dist, next
```

### 3.3 Reconstruct the initial route and recovery plan

``` text
FUNCTION BUILD-PLAN(s, t, next, recoveryPath):

    initialPath = [s]
    recoveryPlan = empty list

    u = s

    WHILE u != t:

        v = next[u]

        IF v == NONE:
            RETURN INFEASIBLE

        append recoveryPath[u][v] to recoveryPlan
        append v to initialPath

        u = v

    RETURN initialPath, recoveryPlan
```

In an implementation, associate each recovery path with the
corresponding initial-path edge `(u,v)`, rather than relying only on
list position. The final destination does not need a blockage-recovery
entry.

------------------------------------------------------------------------

## 4. Correctness proof

### Definitions

For each directed edge choice `(u,v)`, let (R(u,v)) be the minimum cost
of a simple path from `u` to `t` that avoids edge `(u,v)`. This is the
recovery cost obtained from the path index.

Let (D(u)) be the minimum possible worst-case additional travel cost
from `u` to `t`, over all valid strategies under the
at-most-one-blockage assumption.

For a strategy that chooses `v` as the next vertex from `u`, there are
two cases:

-   If `(u,v)` is open, the cost is (w(u,v)+D(v)), because the strategy
    continues from `v`.
-   If `(u,v)` is blocked, the best recovery cost is (R(u,v)).

Therefore, the worst-case cost of choosing `v` is:

\[ `\max`{=tex}`\left`{=tex}(w(u,v)+D(v),R(u,v)`\right`{=tex}). \]

The strategy may choose any neighbour `v`, so the optimal value
satisfies:

\[ D(t)=0 \]

and, for every (u`\ne `{=tex}t),

\[ D(u)= `\min`{=tex}\_{v`\in `{=tex}N(u)}
`\max`{=tex}`\left`{=tex}(w(u,v)+D(v),R(u,v)`\right`{=tex}).
`\tag{1}`{=tex} \]

### Lemma 1 --- The path index gives the correct recovery cost

For any directed edge `(u,v)`, excluding the path-index group
`pathIndex[u][v]` and selecting the cheapest path from the remaining
groups returns (R(u,v)).

**Proof.**

Every simple path from `u` to `t` has exactly one first next vertex. A
path belongs to `pathIndex[u][x]` precisely when its first edge is
`(u,x)`.

A path in `pathIndex[u][v]` uses the blocked edge `(u,v)` immediately,
so it is invalid.

Any simple path in another group `pathIndex[u][x]`, where
(x`\ne `{=tex}v), starts with a different edge. Since the path is
simple, it cannot return to `u`; consequently, it cannot use `(u,v)`
later. It is therefore a valid recovery path.

The remaining groups contain all valid simple recovery paths. Taking the
cheapest candidate among them returns the minimum-cost valid recovery
path, which is (R(u,v)). (`\square`{=tex})

### Lemma 2 --- The minimax recurrence describes the optimal strategy

Equation (1) gives the minimum worst-case cost of a valid strategy from
each vertex.

**Proof.**

Consider any strategy starting at `u`. It must choose some neighbour `v`
as its next vertex.

If `(u,v)` is open, the traveller pays (w(u,v)) and then faces the same
problem from `v`, with at most one blockage still possible. The minimum
achievable worst-case continuation cost is (D(v)).

If `(u,v)` is blocked, the blockage has been used. By the
at-most-one-blockage assumption, every other edge is available, so the
cheapest possible recovery route costs (R(u,v)).

The worst-case cost of choosing `v` is therefore the maximum of these
two cases. Since the strategy can choose whichever neighbour gives the
smallest worst-case cost, minimising over all neighbours gives equation
(1). (`\square`{=tex})

### Lemma 3 --- Reverse-Dijkstra settlement is valid

When the algorithm settles a vertex `u`, its recorded value `dist[u]`
equals (D(u)).

**Proof sketch.**

For a fixed neighbour `v`, define:

\[ F\_{u,v}(x)=`\max`{=tex}`\left`{=tex}(w(u,v)+x,R(u,v)`\right`{=tex}).
\]

This function is non-decreasing in `x`. Also, because edge weights are
positive,

\[ F\_{u,v}(x)`\geq `{=tex}w(u,v)+x\>x. \]

Thus, a candidate value for `u` through `v` is strictly greater than the
value of `v`.

The priority queue processes vertices in non-decreasing tentative
distance. Suppose `u` is the next vertex to be settled, but its
tentative value is larger than its true optimal value. By equation (1),
an optimal choice from `u` uses some neighbour `v`. Since the optimal
cost through `v` is strictly greater than (D(v)), we have (D(v)\<D(u)).
Therefore, `v` must have been settled before `u`.

When `v` was settled, the algorithm examined the edge from `u` to `v`
and performed the corresponding relaxation using the correct value
`dist[v] = D(v)`. It would therefore have assigned `u` a candidate no
greater than its optimal value, contradicting the assumption that `u` is
being settled with a larger value.

The algorithm never assigns a value smaller than the recurrence's
optimum, because every relaxation corresponds to a valid choice of next
vertex and its worst-case cost. Hence, when `u` is settled,
`dist[u]=D(u)`. (`\square`{=tex})

### Theorem --- The algorithm computes an optimal minimax strategy

Assuming the path index contains all required simple paths and every
chosen edge has a feasible recovery route, the algorithm computes the
minimum worst-case travel cost from every vertex to `t`. Following
`next[u]` gives an optimal initial route, and the stored recovery paths
give the corresponding optimal response if an edge on that route is
blocked.

**Proof.**

By Lemma 1, the recovery-cost table contains the correct values
(R(u,v)). By Lemma 2, the optimal strategy values satisfy the minimax
recurrence. By Lemma 3, the reverse-Dijkstra-style algorithm computes
those values exactly.

For each vertex `u`, `next[u]` records a neighbour achieving the minimum
in the recurrence. Since

\[ dist\[u\]`\geq `{=tex}w(u,next\[u\])+dist\[next\[u\]\] \]

and all edge weights are positive, following `next` strictly decreases
`dist`. Therefore, the pointers cannot form a cycle and the route
reaches `t`.

If the selected edge is open, following the next pointer continues with
an optimal strategy. If it is blocked, the stored recovery path has cost
(R(u,v)), which is the cheapest valid recovery route by Lemma 1. Thus,
the resulting strategy achieves the computed minimax value.
(`\square`{=tex})

------------------------------------------------------------------------

## 5. Time and space complexity

The path-enumeration stage can be exponential. The minimax stage is much
cheaper once the path index and recovery costs have been built.

Let:

-   (n=\|V\|): number of original graph vertices.
-   (m=\|E\|): number of original graph edges.
-   (N_T): number of nodes in the path-enumeration tree.
-   (L): total number of vertex occurrences across all enumerated tree
    paths (equivalently, the total path-history length, up to the chosen
    representation).
-   (P): number of simple paths represented by the enumeration.

### 5.1 Path tree and path index

The number of simple paths in a graph can be exponential in (n).
Consequently, (N_T), (P), and (L) can all be exponential in the worst
case.

If path histories are copied into each tree node, construction time
includes the cost of copying those histories. A useful bound is:

\[
O`\left`{=tex}(`\sum`{=tex}\_{x`\in `{=tex}T}`\deg`{=tex}\_G(vertex(x))+L`\right`{=tex}),
\]

where the first term accounts for examining original-graph neighbours
for every tree node and the second accounts for path-history copying.

Storing the tree and copied path histories requires:

\[ O(N_T+L) \]

space, excluding additional implementation overhead.

Building a path index that stores every complete candidate path also
takes space proportional to the total size of those stored paths, which
can be (O(L)) or larger if paths are copied multiple times.

**Practical optimisation:** For recovery-cost calculation, it is
sufficient to keep only the cheapest path in each
`(current vertex, next vertex)` group. Store a reference to its tree
node rather than copying the full path. This avoids retaining every
candidate path in the index, although the path-enumeration tree itself
may still be exponential.

### 5.2 Recovery-cost calculation

Let (g_u) be the number of distinct next-vertex groups for vertex `u`.
The direct pseudocode scans all groups for every possible blocked
outgoing edge. Its time complexity is:

\[
O`\left`{=tex}(`\sum`{=tex}\_{u`\in `{=tex}V}`\deg`{=tex}(u),g_u`\right`{=tex})
\]

after the cheapest candidate in each group is known.

Since (g_u`\leq`{=tex}`\deg`{=tex}(u)), this is at most:

\[
O`\left`{=tex}(`\sum`{=tex}\_{u`\in `{=tex}V}`\deg`{=tex}(u)\^2`\right`{=tex}).
\]

With the optional cheapest/second-cheapest optimisation, the cheapest
group excluding a given next vertex can be found in (O(1)) time after
the group minima have been calculated. All directed-edge recovery costs
can then be prepared in:

\[ O(m) \]

time, because an undirected graph has (2m) directed edge choices.

Reconstructing all stored recovery paths takes time proportional to
their total output length.

### 5.3 Modified reverse Dijkstra

Each undirected edge is considered in both directions, giving (O(m))
relaxations. With a binary min-heap, the time complexity is:

\[ `\boxed{O((n+m)\log n)}`{=tex} \]

and the distance, next-pointer, and priority-queue storage is:

\[ `\boxed{O(n+m)}`{=tex}. \]

This assumes each recovery-cost lookup is (O(1)), as with a precomputed
recovery-cost table or the cheapest/second-cheapest group optimisation.

### 5.4 Overall complexity

The overall cost is dominated by simple-path enumeration:

  -----------------------------------------------------------------------
  Stage                               Time complexity
  ----------------------------------- -----------------------------------
  Build path-enumeration tree         Exponential in the worst case

  Build grouped path index            Proportional to the number/total
                                      size of enumerated paths

  Calculate recovery costs            (O(m)) with
                                      cheapest/second-cheapest group
                                      optimisation

  Modified reverse Dijkstra           (O((n+m)`\log `{=tex}n))

  Reconstruct initial and recovery    Proportional to the total length of
  paths                               the output paths
  -----------------------------------------------------------------------

Therefore, the modified Dijkstra stage is polynomial and relatively
efficient, but the complete algorithm is **not polynomial-time in
general**, because it relies on enumerating simple paths.

------------------------------------------------------------------------

## 6. Important limitations and implementation checks

1.  **At most one blockage:** The recurrence assumes that once a
    blockage is encountered, no other edge can be blocked. It does not
    directly handle (k\>1).
2.  **Recovery paths:** A recovery path is calculated from the current
    vertex and excludes the edge just discovered to be blocked. Its cost
    does not include travel already completed before reaching that
    vertex.
3.  **Simple paths:** The path-index argument relies on candidate paths
    being simple. A path that starts with a different neighbour but
    later returns to `u` could use the blocked edge `(u,v)`; simple
    paths cannot do this.
4.  **Feasibility:** If no path from `u` to `t` avoids `(u,v)`, then
    `recoveryCost[u][v]` is infinity. Such a next-edge choice must not
    be selected if the strategy requires recovery from that blockage.
5.  **Scope of the result:** The algorithm optimises the worst-case cost
    of an adaptive strategy: continue with the selected next vertex if
    the edge works, or take the precomputed recovery route if it is
    blocked. It is not automatically equivalent to every possible
    formulation that first fixes an initial path and then restricts
    recovery choices.
6.  **Exponential preprocessing:** The path tree and full path
    enumeration may become impractical on larger graphs. The minimax
    stage does not remove this limitation.

## 7. Recommended validation

Before relying on the proof in a report, implement a small-graph
exhaustive checker:

1.  Generate small connected weighted undirected graphs.
2.  Enumerate all simple paths and construct the path index.
3.  Run the minimax algorithm.
4.  Independently enumerate possible deterministic next-edge strategies,
    or solve the minimax recurrence by repeated relaxation until
    convergence.
5.  Compare the resulting value at `s` and verify that the extracted
    strategy's worst-case cost matches it.
6.  Test cases with bridges, cycles, equal-cost paths, and vertices with
    only one route to `t`.

This is particularly important because the proof depends on the exact
definition of the strategy and the assumption that at most one blockage
occurs.
