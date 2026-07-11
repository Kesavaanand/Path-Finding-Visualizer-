// ==============================================================
// Pathfind — frontend controller
// ==============================================================
// This file owns NO algorithm logic. Its only jobs are:
//   1. Render whatever grid state the C++ backend sends back
//   2. Forward user interactions (draw wall, run algorithm...) to
//      the backend's REST API
//   3. Animate the visitedOrder / path arrays the backend returns
// ==============================================================

const API_BASE = ''; // same-origin: server serves both API and static files

const CellType = { OPEN: 0, WALL: 1, START: 2, END: 3 };

const state = {
  algorithm: 'bfs',
  drawMode: 'wall',
  isMouseDown: false,
  grid: null,          // last grid JSON from server
  animSpeed: 60,        // 1-100, higher = faster
  isRunning: false,
};

const ALGO_NOTES = {
  bfs: {
    complexity: 'Time: <code>O(V + E)</code> · Space: <code>O(V)</code>',
    text: `Explores the grid ring by ring using a FIFO queue. Every cell costs the
           same to enter, so BFS guarantees the fewest <em>steps</em> — but it will
           walk straight through expensive terrain without noticing.`
  },
  dfs: {
    complexity: 'Time: <code>O(V + E)</code> · Space: <code>O(V)</code>',
    text: `Dives down one branch at a time using a stack, backtracking when it hits
           a dead end. Finds <em>a</em> path fast, but rarely the shortest one —
           watch how much more it winds around compared to BFS.`
  },
  dijkstra: {
    complexity: 'Time: <code>O((V+E) log V)</code> · Space: <code>O(V)</code>',
    text: `Expands the currently-cheapest-known cell first, using a min-heap
           priority queue. Unlike BFS, it correctly routes AROUND weighted
           terrain instead of walking straight through it.`
  },
  astar: {
    complexity: 'Time: <code>O(E)</code> in practice, far fewer nodes than Dijkstra',
    text: `Same min-heap idea as Dijkstra, but adds a Manhattan-distance heuristic
           that "aims" the search at the goal. Notice how much smaller the
           visited region is — that's the heuristic doing its job.`
  }
};

// ---------------- DOM refs ----------------
const gridContainer = document.getElementById('gridContainer');
const gridLoading = document.getElementById('gridLoading');
const connDot = document.getElementById('connStatus');
const connText = document.getElementById('connText');
const speedInput = document.getElementById('speed');
const speedLabel = document.getElementById('speedLabel');

// ---------------- API helpers ----------------
async function apiGet(path) {
  const r = await fetch(API_BASE + path);
  if (!r.ok) throw new Error(`GET ${path} failed`);
  return r.json();
}
async function apiPost(path, body) {
  const r = await fetch(API_BASE + path, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body || {})
  });
  if (!r.ok) throw new Error(`POST ${path} failed`);
  return r.json();
}

// ---------------- Rendering ----------------
function renderGrid(gridData) {
  state.grid = gridData;
  const { rows, cols, cells } = gridData;

  gridContainer.style.gridTemplateColumns = `repeat(${cols}, 1fr)`;
  gridContainer.style.gridTemplateRows = `repeat(${rows}, 1fr)`;
  gridContainer.innerHTML = '';

  const frag = document.createDocumentFragment();
  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols; c++) {
      const cell = cells[r][c];
      const div = document.createElement('div');
      div.className = 'cell';
      div.dataset.r = r;
      div.dataset.c = c;
      applyCellClass(div, cell);
      div.addEventListener('mousedown', () => onCellInteract(r, c));
      div.addEventListener('mouseenter', () => { if (state.isMouseDown) onCellInteract(r, c); });
      frag.appendChild(div);
    }
  }
  gridContainer.appendChild(frag);
}

function applyCellClass(div, cell) {
  div.classList.remove('wall', 'mud', 'start', 'end', 'visited', 'path');
  if (cell.type === CellType.WALL) div.classList.add('wall');
  else if (cell.type === CellType.START) div.classList.add('start');
  else if (cell.type === CellType.END) div.classList.add('end');
  else if (cell.weight > 1) div.classList.add('mud');
}

function cellEl(r, c) {
  return gridContainer.querySelector(`.cell[data-r="${r}"][data-c="${c}"]`);
}

// ---------------- Interaction ----------------
async function onCellInteract(r, c) {
  if (state.isRunning) return;
  let mode = state.drawMode;
  if (mode === 'erase') mode = 'open';
  const data = await apiPost('/api/grid/cell', { r, c, mode });
  renderGrid(data);
}

document.addEventListener('mousedown', () => { state.isMouseDown = true; });
document.addEventListener('mouseup', () => { state.isMouseDown = false; });

document.querySelectorAll('.tool-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    document.querySelectorAll('.tool-btn').forEach(b => b.classList.remove('active'));
    btn.classList.add('active');
    state.drawMode = btn.dataset.mode;
  });
});

document.querySelectorAll('.algo-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    document.querySelectorAll('.algo-btn').forEach(b => {
      b.classList.remove('active');
      b.setAttribute('aria-selected', 'false');
    });
    btn.classList.add('active');
    btn.setAttribute('aria-selected', 'true');
    state.algorithm = btn.dataset.algo;
    const notes = ALGO_NOTES[state.algorithm];
    document.getElementById('algoNotes').innerHTML =
      `<p class="algo-complexity">${notes.complexity}</p><p>${notes.text}</p>`;
  });
});

speedInput.addEventListener('input', () => {
  state.animSpeed = Number(speedInput.value);
  speedLabel.textContent = state.animSpeed > 66 ? 'Fast' : state.animSpeed > 33 ? 'Medium' : 'Slow';
});

document.getElementById('generateMaze').addEventListener('click', async () => {
  if (state.isRunning) return;
  clearVisualOverlay();
  const data = await apiPost('/api/grid/maze', {});
  renderGrid(data);
});

document.getElementById('clearGrid').addEventListener('click', async () => {
  if (state.isRunning) return;
  const rows = state.grid ? state.grid.rows : 25;
  const cols = state.grid ? state.grid.cols : 40;
  const data = await apiPost('/api/grid/new', { rows, cols });
  renderGrid(data);
  resetStats();
});

document.getElementById('runAlgo').addEventListener('click', runAlgorithm);

function clearVisualOverlay() {
  gridContainer.querySelectorAll('.cell.visited, .cell.path').forEach(el => {
    el.classList.remove('visited', 'path');
  });
}

function resetStats() {
  document.getElementById('statVisited').textContent = '—';
  document.getElementById('statPath').textContent = '—';
  document.getElementById('statCost').textContent = '—';
  document.getElementById('statTime').textContent = '—';
}

// ---------------- Run + animate ----------------
async function runAlgorithm() {
  if (state.isRunning) return;
  state.isRunning = true;
  clearVisualOverlay();

  const runBtn = document.getElementById('runAlgo');
  runBtn.disabled = true;
  runBtn.textContent = 'Running…';

  try {
    const result = await apiPost('/api/solve', { algorithm: state.algorithm });
    await animateVisited(result.visitedOrder);
    await animatePath(result.path);

    document.getElementById('statVisited').textContent = result.visitedCount;
    document.getElementById('statPath').textContent = result.path.length ? result.path.length : '0 (no path)';
    document.getElementById('statCost').textContent = result.pathCost >= 0 ? result.pathCost : '—';
    document.getElementById('statTime').textContent = `${result.microseconds} µs`;
  } catch (err) {
    connText.textContent = 'Error contacting backend — is the server running?';
    connDot.classList.remove('online');
  } finally {
    runBtn.disabled = false;
    runBtn.textContent = 'Run ▸';
    state.isRunning = false;
  }
}

function delayForSpeed() {
  // speed 1..100 -> delay 40ms..1ms (inverted, exponential-ish feel)
  const t = state.animSpeed / 100;
  return Math.max(1, Math.round(40 * (1 - t) + 1));
}

function animateVisited(cells) {
  return new Promise(resolve => {
    const delay = delayForSpeed();
    let i = 0;
    // Batch a few cells per frame at high speeds so large grids don't crawl
    const batchSize = state.animSpeed > 80 ? 6 : state.animSpeed > 50 ? 3 : 1;

    function step() {
      const end = Math.min(i + batchSize, cells.length);
      for (; i < end; i++) {
        const [r, c] = cells[i];
        const el = cellEl(r, c);
        if (el && !el.classList.contains('start') && !el.classList.contains('end')) {
          el.classList.add('visited');
        }
      }
      if (i < cells.length) {
        setTimeout(step, delay);
      } else {
        resolve();
      }
    }
    step();
  });
}

function animatePath(cells) {
  return new Promise(resolve => {
    const delay = Math.max(2, delayForSpeed());
    let i = 0;
    function step() {
      if (i < cells.length) {
        const [r, c] = cells[i];
        const el = cellEl(r, c);
        if (el && !el.classList.contains('start') && !el.classList.contains('end')) {
          el.classList.add('path');
        }
        i++;
        setTimeout(step, delay);
      } else {
        resolve();
      }
    }
    step();
  });
}

// ---------------- Boot ----------------
async function boot() {
  try {
    await apiGet('/api/health');
    connDot.classList.add('online');
    connText.textContent = 'connected to C++ backend';

    const data = await apiGet('/api/grid');
    gridLoading.style.display = 'none';
    renderGrid(data);
  } catch (err) {
    connText.textContent = 'backend unreachable — start the server on :8080';
    gridLoading.textContent = 'Could not load grid. Is the backend running?';
  }
}

boot();
