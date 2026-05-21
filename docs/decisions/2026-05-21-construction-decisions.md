# Architecture Decisions — 2026-05-21 (Control SW)

Decisions made during Construction phase iteration 2 that affect the RVC Control SW.

---

## AD-07: onTick() Must Re-Issue FORWARD Every Cycle

**Context**
After `start()` is called, the RVC should move forward continuously until an obstacle is detected or `stop()` is called. The initial implementation issued `FORWARD` only once inside `start()`.

**Decision Factor**
Real H-bridge hardware latches a direction until commanded otherwise, so a single `FORWARD` at startup is sufficient in production. However, the interface contract for `IMotorController` makes no such guarantee — each `move()` call is a discrete command, and callers cannot assume continuity across tick boundaries. The Control SW must express continuous forward intent explicitly on every tick, regardless of the underlying implementation.

**Decision**
`onTick()` issues `_motor->move(Direction::FORWARD)` at the end of every CLEANING or INTENSIFYING tick.

**Output**
- `src/app/RvcController.cpp` — `onTick()` appends FORWARD when `_state == CLEANING || INTENSIFYING`

**Trade-off**
Sends one redundant command per tick to real hardware (which would ignore it, as direction is already latched). Accepted because the interface contract must be self-contained — `RvcController` cannot make assumptions about `IMotorController` implementation details.
