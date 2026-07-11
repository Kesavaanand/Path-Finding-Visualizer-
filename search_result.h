#pragma once
#include <vector>
#include <utility>

// Every algorithm reports:
//  - visitedOrder: cells in the order they were EXPLORED (for animation,
//    and to visually compare how "wasteful" BFS/DFS are vs A*)
//  - path: the reconstructed shortest/found path from start to end
//  - pathCost: sum of cell weights along the path (meaningful for
//    Dijkstra/A*; BFS/DFS just count steps)
struct SearchResult {
    std::vector<std::pair<int,int>> visitedOrder;
    std::vector<std::pair<int,int>> path;
    int pathCost = -1;      // -1 => no path found
    long long microseconds = 0;
};
