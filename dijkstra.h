#pragma once
#include "grid.h"
#include "search_result.h"
#include <queue>
#include <vector>
#include <chrono>
#include <limits>
#include <algorithm>

// ------------------------------------------------------------
// Dijkstra's Algorithm
// ------------------------------------------------------------
// Same idea as BFS, but each cell can cost MORE than 1 to enter
// (our "mud" terrain). A plain queue can't handle that -- a cell
// discovered later via a cheaper route might need to jump ahead
// of one discovered earlier via an expensive route. That's exactly
// what a MIN-HEAP priority queue gives us: always expand the
// currently-cheapest-known cell next.
// This is the concrete, visual reason Dijkstra exists: watch it
// route AROUND mud that BFS walks straight through.
// Time:  O((V + E) log V) with a binary heap
// Space: O(V)
// ------------------------------------------------------------

class Dijkstra {
public:
    static SearchResult run(const Grid& grid, std::pair<int,int> start, std::pair<int,int> end) {
        auto t0 = std::chrono::high_resolution_clock::now();
        SearchResult result;

        int rows = grid.rows(), cols = grid.cols();
        const int INF = std::numeric_limits<int>::max();
        std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, INF));
        std::vector<std::vector<std::pair<int,int>>> parent(
            rows, std::vector<std::pair<int,int>>(cols, {-1,-1}));
        std::vector<std::vector<bool>> settled(rows, std::vector<bool>(cols, false));

        // min-heap of (distance, row, col) -- std::priority_queue is a
        // max-heap by default, so we use std::greater to flip it.
        using State = std::tuple<int,int,int>;
        std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

        dist[start.first][start.second] = 0;
        pq.push({0, start.first, start.second});

        bool found = false;
        while (!pq.empty()) {
            auto [d, r, c] = pq.top(); pq.pop();
            if (settled[r][c]) continue; // stale entry, skip (lazy deletion)
            settled[r][c] = true;
            result.visitedOrder.push_back({r, c});

            if (r == end.first && c == end.second) { found = true; break; }

            for (int dir = 0; dir < 4; dir++) {
                int nr = r + Grid::dr[dir], nc = c + Grid::dc[dir];
                if (!grid.isWalkable(nr, nc) || settled[nr][nc]) continue;
                int newDist = d + grid.at(nr, nc).weight;
                if (newDist < dist[nr][nc]) {
                    dist[nr][nc] = newDist;
                    parent[nr][nc] = {r, c};
                    pq.push({newDist, nr, nc});
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
            result.pathCost = dist[end.first][end.second];
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        result.microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        return result;
    }
};
