// ==============================================================
// Pathfinding Visualizer -- Backend Server
// ==============================================================
// All DSA logic (BFS, DFS, Dijkstra, A*, DSU-based maze generation)
// lives alongside this file. This file is intentionally "dumb": it
// just holds the current grid in memory, exposes it over a small
// REST API, and serves the frontend's static files.
//
// Endpoints:
//   GET  /api/health                -> {"status":"ok"}
//   POST /api/grid/new              -> create a blank rows x cols grid
//   POST /api/grid/maze             -> generate a maze (DSU/Kruskal's)
//   POST /api/grid/cell             -> toggle a single cell (wall/mud/etc)
//   GET  /api/grid                  -> current grid as JSON
//   POST /api/solve                 -> run an algorithm, get the result
// ==============================================================

#include "httplib.h"
#include "json_utils.h"
#include "grid.h"
#include "dsu.h"
#include "maze_generator.h"
#include "bfs.h"
#include "dfs.h"
#include "dijkstra.h"
#include "astar.h"

#include <memory>
#include <mutex>
#include <cstdlib>
#include <iostream>

static std::unique_ptr<Grid> g_grid;
static std::pair<int,int> g_start = {0, 0};
static std::pair<int,int> g_end = {0, 0};
static std::mutex g_mutex;

static void resetGrid(int rows, int cols) {
    g_grid = std::make_unique<Grid>(rows, cols);
    g_grid->at(0, 0).type = CellType::START;
    g_grid->at(rows - 1, cols - 1).type = CellType::END;
    g_start = {0, 0};
    g_end = {rows - 1, cols - 1};
}

int main() {
    resetGrid(25, 40);

    httplib::Server svr;

    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });

    // Serve the frontend as static files -- one deployable service.
    svr.set_mount_point("/", "./public");

    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    svr.Get("/api/grid", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);
        res.set_content(gridToJson(*g_grid), "application/json");
    });

    svr.Post("/api/grid/new", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);
        int rows = extractInt(req.body, "rows", 25);
        int cols = extractInt(req.body, "cols", 40);
        rows = std::max(5, std::min(rows, 80));
        cols = std::max(5, std::min(cols, 120));
        resetGrid(rows, cols);
        res.set_content(gridToJson(*g_grid), "application/json");
    });

    svr.Post("/api/grid/maze", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);
        int seed = extractInt(req.body, "seed", -1);
        unsigned int useSeed = seed >= 0 ? static_cast<unsigned int>(seed)
                                          : std::random_device{}();
        MazeGenerator::generate(*g_grid, useSeed);

        // Recover wherever the generator placed start/end
        for (int r = 0; r < g_grid->rows(); r++)
            for (int c = 0; c < g_grid->cols(); c++) {
                if (g_grid->at(r, c).type == CellType::START) g_start = {r, c};
                if (g_grid->at(r, c).type == CellType::END) g_end = {r, c};
            }

        res.set_content(gridToJson(*g_grid), "application/json");
    });

    // Toggle a cell between OPEN <-> WALL, or set it as mud/start/end.
    // Body: {"r":3,"c":4,"mode":"wall"}  modes: wall, open, mud, start, end
    svr.Post("/api/grid/cell", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);
        int r = extractInt(req.body, "r", -1);
        int c = extractInt(req.body, "c", -1);
        std::string mode = extractString(req.body, "mode", "wall");

        if (!g_grid->inBounds(r, c)) {
            res.status = 400;
            res.set_content("{\"error\":\"out of bounds\"}", "application/json");
            return;
        }

        if (mode == "wall") {
            g_grid->at(r, c) = Cell{CellType::WALL, 1};
        } else if (mode == "open") {
            g_grid->at(r, c) = Cell{CellType::OPEN, 1};
        } else if (mode == "mud") {
            g_grid->at(r, c) = Cell{CellType::OPEN, 5};
        } else if (mode == "start") {
            g_grid->at(g_start.first, g_start.second).type = CellType::OPEN;
            g_grid->at(r, c).type = CellType::START;
            g_start = {r, c};
        } else if (mode == "end") {
            g_grid->at(g_end.first, g_end.second).type = CellType::OPEN;
            g_grid->at(r, c).type = CellType::END;
            g_end = {r, c};
        }

        res.set_content(gridToJson(*g_grid), "application/json");
    });

    // Body: {"algorithm":"bfs"}  algorithm: bfs, dfs, dijkstra, astar
    svr.Post("/api/solve", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);
        std::string algo = extractString(req.body, "algorithm", "bfs");

        SearchResult result;
        if (algo == "bfs") {
            result = BFS::run(*g_grid, g_start, g_end);
        } else if (algo == "dfs") {
            result = DFS::run(*g_grid, g_start, g_end);
        } else if (algo == "dijkstra") {
            result = Dijkstra::run(*g_grid, g_start, g_end);
        } else if (algo == "astar") {
            result = AStar::run(*g_grid, g_start, g_end);
        } else {
            res.status = 400;
            res.set_content("{\"error\":\"unknown algorithm\"}", "application/json");
            return;
        }

        res.set_content(resultToJson(result), "application/json");
    });

    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // Render sets $PORT; default to 8080 for local runs.
    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::atoi(portEnv) : 8080;

    std::cout << "Pathfinding Visualizer server listening on port " << port << std::endl;
    svr.listen("0.0.0.0", port);

    return 0;
}
