# Simulator / UI Failures and Resolutions — 2026-05-21

Failures in Simulator and UI components encountered during Construction iteration 2. Separate from RVC Control SW failures.

---

## F-05: Terminal Display/Input Collision ("밀림 현상")

### What Failed
After adding a background tick thread, the terminal display became corrupted whenever the user typed. Typed characters appeared inside the grid, and the cursor jumped to unpredictable positions. The display was unusable.

### Root Cause
Two independent writes raced to the same terminal:

1. The tick thread called `std::cout` to render the grid (including `\033[H` to home the cursor).
2. The terminal kernel echoed typed characters to stdout at the current cursor position.

Because the tick thread repositioned the cursor mid-render, typed characters appeared wherever the cursor happened to be at that instant.

### Why It Was Missed
In the single-threaded scripted demo, rendering and input were never concurrent. Moving to a multithreaded design exposed the terminal cursor as shared mutable state that both threads implicitly modified.

### Resolution
Switched to **termios raw mode** (see AD-08):

1. Disabled kernel echo and line buffering via `tcsetattr`.
2. Application manages an explicit input string buffer.
3. After every render, cursor is repositioned to a fixed input row and `> <buffer>` is printed.
4. Main thread reads one character at a time — no kernel echo to arbitrary positions.
5. Both threads access stdout only while holding `sim_mutex`.

### Why Not ncurses
ncurses would solve this cleanly but is prohibited by project rules. `termios` is POSIX libc — no installation or CMake changes required.

### Prevention
Any application that mixes concurrent stdout writes with user input must either use a terminal UI library or take explicit control via raw mode. Default line-discipline echo is incompatible with threaded rendering.
