#pragma once
#include <vector>
#include <cstdint>

// ------------------------------------------------------------
// Grid
// ------------------------------------------------------------
// The grid IS the graph. Every cell is a vertex; an edge exists
// between orthogonally-adjacent cells that are not walls.
// Cell "weight" lets us model non-uniform terrain cost (e.g. mud),
// which is what makes Dijkstra/A* meaningfully different from BFS.
// ------------------------------------------------------------

enum class CellType : uint8_t {
    OPEN = 0,
    WALL = 1,
    START = 2,
    END = 3
};

struct Cell {
    CellType type = CellType::OPEN;
    int weight = 1; // 1 = normal, 5 = "mud"/difficult terrain
};

class Grid {
public:
    Grid(int rows, int cols) : rows_(rows), cols_(cols), cells_(rows * cols) {}

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    inline int index(int r, int c) const { return r * cols_ + c; }

    Cell& at(int r, int c) { return cells_[index(r, c)]; }
    const Cell& at(int r, int c) const { return cells_[index(r, c)]; }

    bool inBounds(int r, int c) const {
        return r >= 0 && r < rows_ && c >= 0 && c < cols_;
    }

    bool isWalkable(int r, int c) const {
        return inBounds(r, c) && at(r, c).type != CellType::WALL;
    }

    // 4-directional neighbors: this is what makes it a graph problem,
    // not just a 2D array problem. Order = up, right, down, left.
    static constexpr int dr[4] = {-1, 0, 1, 0};
    static constexpr int dc[4] = {0, 1, 0, -1};

private:
    int rows_, cols_;
    std::vector<Cell> cells_;
};
