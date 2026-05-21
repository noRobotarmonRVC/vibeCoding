# Failures and Resolutions — 2026-05-21 (Control SW)

Failures in RVC Control SW encountered during Construction iteration 2.

---

## F-04: RVC Does Not Move After start()

### What Failed
After calling `start()` and running multiple ticks, the robot remained at its initial position. Only the Tick counter incremented. The motor log showed a single `FORWARD` entry from `start()` but nothing after.

### Root Cause
`RvcController::start()` issued `_motor->move(Direction::FORWARD)` once. `RvcController::onTick()` never issued another motor command while in the CLEANING state — it only checked the dust sensor and returned.

The `IMotorController` interface makes no guarantee that a direction persists across ticks. Each `move()` is a discrete command. Once consumed, the FORWARD issued by `start()` was gone, and subsequent ticks produced no new motor output.

### Why It Was Missed
The design assumed that "issuing FORWARD once puts the motor in a continuous forward state," which holds for real H-bridge hardware. The Control SW incorrectly relied on an implementation detail of the hardware rather than honoring the interface contract, which is implementation-agnostic.

### Resolution
Added `_motor->move(Direction::FORWARD)` at the end of `onTick()` whenever `_state == CLEANING || _state == INTENSIFYING`:

```cpp
if (_state == RvcState::CLEANING || _state == RvcState::INTENSIFYING) {
    _motor->move(Direction::FORWARD);
}
```

### Prevention
`RvcController` must treat `IMotorController` as a stateless command sink. Continuous behaviors must be re-commanded every tick. The interface contract — not hardware knowledge — defines what the controller may assume.
