#pragma once
#include "grid.h"
#include "search_result.h"
#include <queue>
#include <vector>
#include <chrono>

// ------------------------------------------------------------
// Breadth-First Search
// ------------------------------------------------------------
// Explores the grid in "rings" of increasing distance using a FIFO
// queue. Guarantees the shortest path in terms of NUMBER OF STEPS,
// but is blind to cell weight -- it will happily walk straight
// through expensive "mud" terrain because it only counts hops.
// Time:  O(V + E)  ~ O(rows*cols) on a grid
// Space: O(V)
// ------------------------------------------------------------

class BFS {
public:
    static SearchResult run(const Grid& grid, std::pair<int,int> start, std::pair<int,int> end) {
        auto t0 = std::chrono::high_resolution_clock::now();
        SearchResult result;

        int rows = grid.rows(), cols = grid.cols();
        std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
        std::vector<std::vector<std::pair<int,int>>> parent(
            rows, std::vector<std::pair<int,int>>(cols, {-1,-1}));

        std::queue<std::pair<int,int>> q;
        q.push(start);
        visited[start.first][start.second] = true;

        bool found = false;
        while (!q.empty()) {
            auto [r, c] = q.front(); q.pop();
            result.visitedOrder.push_back({r, c});

            if (r == end.first && c == end.second) { found = true; break; }

            for (int d = 0; d < 4; d++) {
                int nr = r + Grid::dr[d], nc = c + Grid::dc[d];
                if (grid.isWalkable(nr, nc) && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    parent[nr][nc] = {r, c};
                    q.push({nr, nc});
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
