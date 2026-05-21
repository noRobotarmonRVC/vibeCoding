# Simulator / UI Decisions — 2026-05-21

Decisions made during Construction phase iteration 2 that affect the Simulator and UI components only. These are separate from the RVC Control SW architecture.

---

## AD-08: termios Raw Mode for Split Display / Input

**Context**
Adding a background tick thread means two threads write to the same terminal: the tick thread writes the grid, and the main thread reads user input. Terminal echo writes typed characters wherever the cursor happens to be, causing display corruption.

**Decision Factor**
Three options were considered:

| Option | Mechanism | Verdict |
|---|---|---|
| Accept the artifact | No change | Unusable UI |
| ncurses | External library | Prohibited by project rules |
| termios raw mode | POSIX standard (no install required) | Selected |

**Decision**
Enable raw mode (`~ECHO | ~ICANON`) via `termios`. Maintain a string input buffer manually. After every render, reposition the cursor to a fixed input row (`grid_h + 4`) and redraw `> <buffer>`. Restore the terminal on exit via `std::atexit`.

**Output**
- `src/app/main.cpp` — `enableRawMode()`, `restoreTerminal()`, `renderInputLine()`

**Trade-off**
Arrow-key history and readline-style editing are not available. Accepted for a demo-level CLI.

---

## AD-09: Background Tick Thread with std::mutex

**Context**
The UI requires the simulation to advance automatically at a configurable rate while the user types commands concurrently.

**Decision**
Spawn one `std::thread` that loops: sleep `tick_ms` ms → lock `sim_mutex` → `sim.tick()` → render → unlock. The main thread acquires the same mutex before any `Simulator` call, serializing all access.

**Output**
- `src/app/main.cpp` — `tick_thread` lambda, `std::mutex sim_mutex`, `std::atomic<int> tick_ms`
- `speed <ms>` command updates `tick_ms` atomically without stopping the thread

**Trade-off**
User commands block for up to one tick interval. At 300 ms default this is imperceptible.

---

## AD-10: Grid-Based Dust Placement with Auto-Detection and Consumption

**Context**
The original dust interface was injection-only (`sim.injectDust(true)`), requiring manual timing and giving no persistent visual feedback. Obstacles use a persistent grid model; dust should be consistent.

**Decision**
Add `_dust_cells : std::set<pair<int,int>>` to `Simulator`. In `tick()`, if the robot's current position is in `_dust_cells`, inject `true` into the dust sensor and erase the cell. `GridDisplay` receives `dustCells()` and renders them as cyan `*`.

**Output**
- `src/simulator/Simulator.hpp/cpp` — `placeDust(x,y)`, `dustCells()`, auto-detection in `tick()`
- `src/ui/GridDisplay.hpp/cpp` — `dust_cells` parameter, cyan `*` rendering
- `src/app/main.cpp` — `dust x y` command

**Trade-off**
Manual `injectDust(bool)` is kept for test scenarios that need precise per-tick control.
