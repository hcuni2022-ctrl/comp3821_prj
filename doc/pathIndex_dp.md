#Potential alternative: DP-based path index construction
On 
- Investigate whether dynamic programming can construct pathIndex[current][next] directly from the original graph, without explicitly enumerating every simple path.
- Identify overlapping subproblems and reuse previously calculated path information.
- Preserve enough information to distinguish alternative routes and support recovery when a particular edge is blocked.
- Compare its memory and construction time against the current graph → path-enumeration tree → directed path index approach.