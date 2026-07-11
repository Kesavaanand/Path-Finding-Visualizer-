#pragma once
#include "grid.h"
#include "search_result.h"
#include <stack>
#include <vector>
#include <chrono>
#include <algorithm>

// ------------------------------------------------------------
// Depth-First Search
// ------------------------------------------------------------
// Explores as far as possible down one branch (using an explicit
// stack, to avoid recursion-depth issues on large grids) before
// backtracking. Finds *a* path, not necessarily the shortest one --
// this is the key teaching contrast against BFS.
// Time:  O(V + E)
// Space: O(V)
// ------------------------------------------------------------

class DFS {
public:
    static SearchResult run(const Grid& grid, std::pair<int,int> start, std::pair<int,int> end) {
        auto t0 = std::chrono::high_resolution_clock::now();
        SearchResult result;

        int rows = grid.rows(), cols = grid.cols();
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::vector<std::vector<std::pair<int,int>>> parent(
            rows, std::vector<std::pair<int,int>>(cols, {-1,-1}));

        std::stack<std::pair<int,int>> st;
        st.push(start);

        bool found = false;
        while (!st.empty()) {
            auto [r, c] = st.top(); st.pop();
            if (visited[r][c]) continue;
            visited[r][c] = true;
            result.visitedOrder.push_back({r, c});

            if (r == end.first && c == end.second) { found = true; break; }

            for (int d = 0; d < 4; d++) {
                int nr = r + Grid::dr[d], nc = c + Grid::dc[d];
                if (grid.isWalkable(nr, nc) && !visited[nr][nc]) {
                    if (parent[nr][nc] == std::make_pair(-1, -1))
                        parent[nr][nc] = {r, c};
                    st.push({nr, nc});
                }
            }
        }

        if (found) {
            std::vector<std::pair<int,int>> path;
            auto cur = end;
            int cost = 0;
            while (cur != start) {
                path.push_back(cur);
                cost += grid.at(cur.first, cur.second).weight;
                cur = parent[cur.first][cur.second];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            result.path = path;
            result.pathCost = cost;
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        result.microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        return result;
    }
};
