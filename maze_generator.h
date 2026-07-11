#pragma once
#include "grid.h"
#include "dsu.h"
#include <vector>
#include <random>
#include <algorithm>

// ------------------------------------------------------------
// Maze Generation: Randomized Kruskal's Algorithm
// ------------------------------------------------------------
// Standard Kruskal's builds a Minimum Spanning Tree by sorting all
// edges by weight and greedily adding the cheapest edge that doesn't
// form a cycle (checked via DSU). Here every "edge" (wall between
// two cells) has equal weight, so we shuffle the edge list instead
// of sorting by weight -> a *random* spanning tree -> a maze with
// exactly one solution path between any two cells, no loops.
//
// Talking point for your instructor: this is the same DSU + "add
// edge only if it doesn't connect two already-connected components"
// idea used in Kruskal's MST — just applied to maze walls instead
// of network cables.
// ------------------------------------------------------------

class MazeGenerator {
public:
    struct Edge { int r1, c1, r2, c2; };

    static void generate(Grid& grid, unsigned int seed = std::random_device{}()) {
        int rows = grid.rows(), cols = grid.cols();

        // Start with every cell walled off from its neighbors
        for (int r = 0; r < rows; r++)
            for (int c = 0; c < cols; c++)
                grid.at(r, c).type = CellType::OPEN; // cells themselves stay open;
                                                       // "walls" are logical edges we track

        // We model walls as a separate boolean grid between cells,
        // but to keep the Grid class simple (cell-based, not edge-based),
        // we instead carve a maze onto a grid where odd rows/cols are
        // permanently wall "mortar" and even rows/cols are rooms.
        // This is the standard trick for cell-based maze rendering.
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                bool isWallCell = (r % 2 == 1) || (c % 2 == 1);
                grid.at(r, c).type = isWallCell ? CellType::WALL : CellType::OPEN;
            }
        }

        int roomRows = (rows + 1) / 2;
        int roomCols = (cols + 1) / 2;
        DSU dsu(roomRows * roomCols);
        auto roomIndex = [&](int rr, int rc) { return rr * roomCols + rc; };

        // Build every candidate edge between adjacent "rooms"
        std::vector<Edge> edges;
        for (int rr = 0; rr < roomRows; rr++) {
            for (int rc = 0; rc < roomCols; rc++) {
                if (rc + 1 < roomCols) edges.push_back({rr, rc, rr, rc + 1});
                if (rr + 1 < roomRows) edges.push_back({rr, rc, rr + 1, rc});
            }
        }

        std::mt19937 rng(seed);
        std::shuffle(edges.begin(), edges.end(), rng);

        // Randomized Kruskal's: knock down a wall only if it connects
        // two rooms that aren't already connected (prevents cycles)
        for (const auto& e : edges) {
            int a = roomIndex(e.r1, e.c1);
            int b = roomIndex(e.r2, e.c2);
            if (dsu.unite(a, b)) {
                int gridR = e.r1 + e.r2; // even+even -> even, or even+odd... derive wall cell
                int gridC = e.c1 + e.c2;
                // wall cell sits directly between the two rooms
                int wallR = e.r1 * 2 + (e.r2 - e.r1);
                int wallC = e.c1 * 2 + (e.c2 - e.c1);
                if (grid.inBounds(wallR, wallC)) {
                    grid.at(wallR, wallC).type = CellType::OPEN;
                }
            }
        }

        // Carve start (top-left room) and end (bottom-right room)
        grid.at(0, 0).type = CellType::START;
        int endR = (roomRows - 1) * 2;
        int endC = (roomCols - 1) * 2;
        grid.at(endR, endC).type = CellType::END;

        // Sprinkle some "mud" (weighted terrain) on ~8% of open cells
        // so weighted algorithms (Dijkstra/A*) have something to prove.
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        std::uniform_int_distribution<int> weightDist(4, 6);
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (grid.at(r, c).type == CellType::OPEN && chance(rng) < 0.08) {
                    grid.at(r, c).weight = weightDist(rng);
                }
            }
        }
    }
};
