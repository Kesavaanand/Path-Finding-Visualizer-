# Pathfind — Graph Algorithms Lab

An interactive pathfinding visualizer with a **C++ backend** doing all the real
algorithmic work, and a lightweight HTML/CSS/JS frontend that renders and
animates it. Draw walls, sprinkle weighted terrain, generate a random maze,
and watch BFS, DFS, Dijkstra, and A* race across the same grid.

---

## 1. Why this project shows a lot of DSA

| Concept | File | What it demonstrates |
|---|---|---|
| Graph modeling | `backend/algorithms/grid.h` | A 2D grid treated as an implicit graph — cells are vertices, adjacency is edges |
| BFS (queue) | `backend/algorithms/bfs.h` | Unweighted shortest path, O(V+E) |
| DFS (stack) | `backend/algorithms/dfs.h` | Depth-first traversal + backtracking, finds *a* path not the shortest |
| Dijkstra's algorithm | `backend/algorithms/dijkstra.h` | Weighted shortest path using a **min-heap priority queue** |
| A* search | `backend/algorithms/astar.h` | Dijkstra + Manhattan-distance heuristic — fewer nodes explored |
| Disjoint Set Union (Union-Find) | `backend/algorithms/dsu.h` | Path compression + union by rank, ~O(α(n)) per operation |
| Randomized Kruskal's algorithm | `backend/algorithms/maze_generator.h` | Uses the DSU to generate a perfect maze (spanning tree, no cycles) |

**The live demo story for your instructor:**
1. Generate a maze — explain it's really Kruskal's MST algorithm, using DSU to avoid cycles, just applied to walls instead of network edges.
2. Run BFS — shortest path in *steps*, ignores terrain cost.
3. Run DFS — same start/end, visibly different (usually longer, "wandering") path.
4. Paint some weighted "terrain" cells, run BFS again — it walks straight through them, because BFS can't see weight.
5. Run Dijkstra — routes around the expensive terrain, correct weighted shortest path.
6. Run A* — same optimal path as Dijkstra, but the stats panel shows it visited noticeably fewer cells, because the heuristic focuses the search toward the goal.

That progression (unweighted → weighted → weighted+heuristic) is a genuinely strong narrative and touches almost every core graph-traversal idea taught in a DSA course.

---

## 2. Project structure

```
pathfinding-visualizer/
├── backend/
│   ├── main.cpp                  # HTTP server + routes (uses cpp-httplib)
│   ├── httplib.h                 # vendored single-header HTTP library
│   ├── json_utils.h              # tiny JSON (de)serialization helpers
│   ├── CMakeLists.txt
│   └── algorithms/
│       ├── grid.h                # the graph representation
│       ├── dsu.h                 # Disjoint Set Union
│       ├── maze_generator.h      # DSU-based maze generation
│       ├── search_result.h       # shared result struct
│       ├── bfs.h
│       ├── dfs.h
│       ├── dijkstra.h
│       └── astar.h
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── script.js
├── Dockerfile                    # builds backend + bundles frontend, for Render
├── render.yaml                   # Render Blueprint (optional, one-click config)
└── .gitignore
```

The C++ server does double duty: it exposes the algorithm API **and** serves
the frontend's static files, so the whole thing deploys as a single Render
web service — no CORS headaches, no second service to manage.

---

## 3. Running it locally

You need a C++17 compiler. No external dependencies beyond the vendored
`httplib.h` (already included).

```bash
cd backend
g++ -std=c++17 -O2 -pthread main.cpp -o server
cp -r ../frontend ./public   # server serves ./public as static files
./server
```

Open `http://localhost:8080` in your browser.

(Or with CMake: `cmake -S backend -B build && cmake --build build`, then run
the resulting `server` binary from a directory containing a `public/` folder
with the frontend files.)

---

## 4. Pushing to GitHub

```bash
cd pathfinding-visualizer
git init
git add .
git commit -m "Pathfinding visualizer: BFS/DFS/Dijkstra/A* + DSU maze generation"
```

Create an empty repo on GitHub (no README/license, so it doesn't conflict),
then:

```bash
git remote add origin https://github.com/<your-username>/pathfinding-visualizer.git
git branch -M main
git push -u origin main
```

---

## 5. Deploying on Render

**Option A — Blueprint (fastest):**
1. Push the repo to GitHub (above). `render.yaml` is already in the repo root.
2. In the Render dashboard: **New → Blueprint**, connect your GitHub repo.
3. Render reads `render.yaml` automatically and provisions a Docker web
   service. Click **Apply**.

**Option B — Manual web service:**
1. Render dashboard → **New → Web Service** → connect your GitHub repo.
2. **Runtime:** Docker (Render will detect the `Dockerfile` automatically).
3. **Instance type:** Free is fine for a class demo.
4. Leave the build/start commands blank — the `Dockerfile`'s `CMD` handles it.
5. Click **Create Web Service**. Render builds the image (installs
   build-essential + cmake, compiles the server, copies the frontend into
   `./public`) and deploys it.
6. Once live, Render gives you a URL like
   `https://pathfinding-visualizer.onrender.com` — that's your whole app,
   frontend and backend both served from the same C++ binary.

**Note on the free tier:** free Render web services spin down after a period
of inactivity and take ~30-50 seconds to wake back up on the next request.
For a live instructor demo, open the URL a minute or two beforehand so it's
already warm.

---

## 6. Explaining the code to your instructor — talking points

- **Why a grid is a graph:** each cell is a vertex; up/down/left/right
  movement to a non-wall neighbor is an edge. `Grid::dr`/`Grid::dc` in
  `grid.h` encode the 4 possible edges from any vertex.
- **Why BFS needs a queue, not a stack:** a queue processes cells in the
  order they were *discovered*, which is exactly what guarantees you explore
  everything at distance `d` before anything at distance `d+1` — that's what
  makes the first time you reach the target the *shortest* path.
- **Why Dijkstra needs a priority queue:** once cells can cost different
  amounts to enter, "discovered first" no longer means "cheapest so far." A
  min-heap always lets you expand the cheapest known cell next, which is the
  core loop invariant that makes Dijkstra correct.
- **Why A* is admissible:** the Manhattan distance heuristic never
  *overestimates* the true remaining distance on a 4-directional grid with
  minimum step cost 1 — that's the mathematical property that keeps A*'s
  answer optimal while still exploring less of the graph than Dijkstra.
- **Why maze generation uses DSU:** a "perfect" maze is a spanning tree over
  the grid's rooms (no cycles, every room reachable). Randomized Kruskal's
  algorithm — shuffle candidate edges, add an edge only if `union()` reports
  it connected two previously-separate components — builds exactly that,
  using the same DSU your instructor likely already taught for Kruskal's MST.

---

## 7. Possible extensions if you want to go further

- Add diagonal movement (8-directional) and compare heuristics (Chebyshev vs Manhattan)
- Add a "greedy best-first search" mode (heuristic only, no g-score) to contrast against A*
- Persist custom-drawn grids (save/load) using simple in-memory or file-based storage
- Add Bidirectional BFS for another complexity comparison
