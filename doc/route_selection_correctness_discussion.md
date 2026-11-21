# Section 4.3-4.4: Correctness and History State

## 4.3 Current recurrence and modified reverse-Dijkstra

### Claim 1

For the recurrence

\[
Q(u,v)=\max\left(w(u,v)+D(v),R(u,v)\right),
\qquad
D(u)=\min_{v\in N(u)}Q(u,v),
\qquad D(t)=0,
\]

modified reverse-Dijkstra, using `dist[v]` to relax the corresponding neighbours of `v`, computes the value of this recurrence once `R(u,v)` and the graph data are fixed.

**Proof.** This claim concerns only the algorithm's calculation of the recurrence; it does not claim that the recurrence models the traveller's complete history-dependent problem.

For a fixed neighbour `v`, define

\[
F_{u,v}(x)=\max\left(w(u,v)+x,R(u,v)\right).
\]

This function is non-decreasing in `x`. Because every edge weight is positive,

\[
F_{u,v}(x)\geq w(u,v)+x>x.
\]

Thus, a candidate at `u` obtained through `v` is strictly greater than the settled value at `v`. Consequently, processing vertices in non-decreasing tentative distance gives a label-setting ordering: whenever the algorithm settles `u`, every neighbour `v` that could give the minimum recurrence value must have settled already, since

\[
D(v)<F_{u,v}(D(v))=Q(u,v).
\]

When `v` settles, reverse-Dijkstra examines each neighbour `u` and calculates `max(w(u,v)+dist[v],R(u,v))`. It therefore assigns `u` the minimum over all choices whose neighbour values have already been established. On the other hand, an update can never assign a value smaller than the recurrence's minimum, because every update corresponds to one available neighbour and one valid choice `v`. By induction on the priority-queue settlement order, no vertex can be settled before all of its relevant predecessor choices are known, and when `u` is settled its value equals the recurrence value `D(u)`.

**Conclusion.** Modified reverse-Dijkstra is valid as a solver for the stated recurrence, provided the recovery-cost table `R(u,v)` is finite and correct for that recurrence.

### Claim 2

The recurrence is a representation of the optimal worst-case strategy from every vertex.

**This claim is not established and may be false.** The recurrence's success branch, `w(u,v)+D(v)`, and its failure branch, `R(u,v)`, are evaluated in different knowledge states, so their combination is not automatically the worst case of a single consistent strategy.

The following assumptions are valid in the original problem:

- The graph is connected and undirected.
- Edge weights are positive.
- At most one edge is blocked.
- The traveller learns that an edge is open after successfully traversing it.
- Once a blockage is encountered, no further blockage occurs.
- The cost of recovery from `u` after edge `(u,v)` is blocked is `R(u,v)=d_{G-(u,v)}(u,t)`.

Under these assumptions, consider the five-vertex graph

| Edge | Weight |
|---|---:|
| `(0,1)` | 8 |
| `(0,2)` | 7 |
| `(2,3)` | 11 |
| `(3,4)` | 11 |
| `(0,3)` | 1 |
| `(0,4)` | 8 |
| `(2,4)` | 7 |

with destination `t=4`.

Vertex `1` has only one neighbour, `0`. If no edge is known open, the recurrence has no finite way to handle attempting `(1,0)`, because removing `(1,0)` isolates vertex `1` and gives

\[
R(1,0)=d_{G-(1,0)}(1,4)=\infty.
\]

Hence the unconditioned value is `D(1)=infinity`.

Now suppose the traveller reaches vertex `1` by successfully traversing `(0,1)`. The edge `(0,1)` is known open. The traveller can therefore return along it, pay `8`, and use the route from vertex `0` to vertex `4`. The route `0-3-4` has cost `1+11=12`, so the conditioned value is at most

\[
D(1\mid(0,1)\text{ known open})=8+12=20.
\]

The resulting state-dependent values differ:

\[
D(1)=\infty,
\qquad
D(1\mid(0,1)\text{ known open})=20.
\]

This is not an artificial modification of the assumptions. It is a valid execution of the problem: vertex `1` is infeasible if `(1,0)` may fail, but it is feasible once `(0,1)` has just been confirmed open.

**Conclusion.** Claim 2 is false under the original recurrence because an unconditioned `D(v)` can include a failure on an edge that is already known open after arriving at `v`, and can also assign an infinite cost to a state that is finite under the known-open history.

## 4.4 History-dependent state

### Claim 3

The reverse-Dijkstra state `dist[v]` is sufficient because the only relevant information is that at most one future blockage remains.

**This claim is false.** Traversal history can change the set of edges that may still be blocked. In particular, every successfully traversed edge is known open, whereas `dist[v]` was defined without any such record.

After selecting `(u,v)`:

- In the successful branch, `(u,v)` is known open.
- In the failure branch, `(u,v)` is known blocked and recovery begins immediately.
- In the subsequent successful-branch state, at least one edge, `(u,v)`, must no longer be treated as a possible future blockage.

Therefore `dist[v]` must be regarded as

\[
D(v,\varnothing),
\]

the value with no previously confirmed-open edges, rather than as the value for the state reached after traversing `(u,v)`.

The correct state is

\[
D(v,S),
\]

where `S` is the set of edges confirmed open. For a neighbour `x`, the state transition is

\[
D(u,S)=
\min_{x\in N(u)}
\begin{cases}
w(u,x)+D(x,S),
& (u,x)\in S,\\
\max\left(w(u,x)+D(x,S\cup\{(u,x)\}),R(u,x)\right),
& (u,x)\notin S.
\end{cases}
\]

The first case is the exact success branch for an already-confirmed-open edge. It does not consider a future failure on that edge. The second case applies when the edge has not yet been confirmed open and retains the two-outcome maximisation.

This state still needs to address a second history effect: the successful route may record more than one open edge. Hence recording only whether a blockage has already occurred is not sufficient. The set of open edges must be carried through the continuation.

**Conclusion.** The present `dist[u]` recurrence is not globally justified without this state; the recovered value is valid for a fixed unconditioned recurrence, but it need not be the minimum worst-case value for the actual history-dependent traveller.
