#pragma once
#include "grid.h"
#include "search_result.h"
#include <queue>
#include <vector>
#include <chrono>
#include <limits>
#include <cmath>
#include <algorithm>

// ------------------------------------------------------------
// A* Search
// ------------------------------------------------------------
// Dijkstra explores equally in every direction because it only
// knows the cost SO FAR (g). A* adds a heuristic h(n) estimating
// the remaining cost to the goal, and always expands the cell
// with the smallest f(n) = g(n) + h(n). This "aims" the search
// toward the goal instead of flooding outward uniformly.
//
// We use Manhattan distance |dr| + |dc| as the heuristic, which is
// ADMISSIBLE (never overestimates) on a 4-directional grid with
// minimum step cost 1 -- this is what guarantees A* still finds
// the optimal path, not just *a* path.
// Time:  O(E) in the worst case, but in practice explores far
//        fewer nodes than Dijkstra -- the point you'll demo live.
// Space: O(V)
// ------------------------------------------------------------

class AStar {
public:
    static SearchResult run(const Grid& grid, std::pair<int,int> start, std::pair<int,int> end) {
        auto t0 = std::chrono::high_resolution_clock::now();
        SearchResult result;

        int rows = grid.rows(), cols = grid.cols();
        const int INF = std::numeric_limits<int>::max();
        std::vector<std::vector<int>> gScore(rows, std::vector<int>(cols, INF));
        std::vector<std::vector<std::pair<int,int>>> parent(
            rows, std::vector<std::pair<int,int>>(cols, {-1,-1}));
        std::vector<std::vector<bool>> settled(rows, std::vector<bool>(cols, false));

        auto heuristic = [&](int r, int c) {
            return std::abs(r - end.first) + std::abs(c - end.second);
        };

        // (fScore, row, col) min-heap
        using State = std::tuple<int,int,int>;
        std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

        gScore[start.first][start.second] = 0;
        pq.push({heuristic(start.first, start.second), start.first, start.second});

        bool found = false;
        while (!pq.empty()) {
            auto [f, r, c] = pq.top(); pq.pop();
            if (settled[r][c]) continue;
            settled[r][c] = true;
            result.visitedOrder.push_back({r, c});

            if (r == end.first && c == end.second) { found = true; break; }

            for (int dir = 0; dir < 4; dir++) {
                int nr = r + Grid::dr[dir], nc = c + Grid::dc[dir];
                if (!grid.isWalkable(nr, nc) || settled[nr][nc]) continue;
                int tentativeG = gScore[r][c] + grid.at(nr, nc).weight;
                if (tentativeG < gScore[nr][nc]) {
                    gScore[nr][nc] = tentativeG;
                    parent[nr][nc] = {r, c};
                    int fScore = tentativeG + heuristic(nr, nc);
                    pq.push({fScore, nr, nc});
                }
            }
        }

        if (found) {
            std::vector<std::pair<int,int>> path;
            auto cur = end;
            while (cur != start) {
                path.push_back(cur);
                cur = parent[cur.first][cur.second];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            result.path = path;
            result.pathCost = gScore[end.first][end.second];
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        result.microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        return result;
    }
};
