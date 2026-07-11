#pragma once
#include <vector>
#include <numeric>

// ------------------------------------------------------------
// Disjoint Set Union (Union-Find)
// ------------------------------------------------------------
// This is the classic DSA structure behind Kruskal's MST algorithm.
// We repurpose it here to GENERATE mazes: treat every cell as its
// own set, then repeatedly union() two randomly-chosen adjacent
// cells (knocking down the wall between them) as long as they are
// NOT already in the same set. This guarantees:
//   1. The result is a spanning tree (no cycles) -> exactly one
//      path between any two cells -> a "perfect" maze.
//   2. Union by rank + path compression gives near O(1) amortized
//      operations (technically O(alpha(n)), inverse Ackermann).
// ------------------------------------------------------------

class DSU {
public:
    explicit DSU(int n) : parent_(n), rank_(n, 0) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    int find(int x) {
        // Path compression: flatten the tree as we walk it
        if (parent_[x] != x) {
            parent_[x] = find(parent_[x]);
        }
        return parent_[x];
    }

    // Returns true if a union happened (i.e. x and y were in different sets)
    bool unite(int x, int y) {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;

        // Union by rank: attach smaller tree under bigger tree
        if (rank_[rx] < rank_[ry]) std::swap(rx, ry);
        parent_[ry] = rx;
        if (rank_[rx] == rank_[ry]) rank_[rx]++;
        return true;
    }

    bool connected(int x, int y) { return find(x) == find(y); }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
};
