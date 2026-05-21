# Simulator / UI 결정 사항 — 2026-05-21

Construction 단계 2차 이터레이션에서 Simulator 및 UI 컴포넌트에만 영향을 미치는 결정. RVC Control SW 아키텍처와 별개입니다.

---

## AD-08: 화면/입력 분리를 위한 termios 로우 모드

**배경**
백그라운드 틱 스레드 추가 후, 두 스레드가 동일한 터미널에 동시에 쓰면서 화면이 밀리는 현상이 발생했다.

**결정 근거**

| 선택지 | 방식 | 결과 |
|---|---|---|
| 현상 수용 | 변경 없음 | 사용 불가 |
| ncurses | 외부 라이브러리 | 프로젝트 규칙상 불가 |
| termios 로우 모드 | POSIX 표준 (설치 불필요) | 채택 |

**결정**
`termios`로 로우 모드(`~ECHO | ~ICANON`)를 활성화한다. 입력 버퍼를 수동 관리하고, 매 렌더링 후 커서를 고정 입력 행(`grid_h + 4`)으로 이동시켜 `> <buffer>`를 다시 그린다. `std::atexit`으로 종료 시 터미널 복원.

**산출물**
- `src/app/main.cpp` — `enableRawMode()`, `restoreTerminal()`, `renderInputLine()`

**트레이드오프**
방향키 히스토리, readline 편집 미지원. 데모 수준 CLI이므로 허용.

---

## AD-09: std::mutex를 이용한 백그라운드 틱 스레드

**배경**
시뮬레이션이 자동으로 진행되면서 동시에 사용자가 명령을 입력할 수 있어야 한다.

**결정**
`std::thread`를 생성하여 루프 실행: `tick_ms` ms 대기 → `sim_mutex` 잠금 → `sim.tick()` → 렌더링 → 잠금 해제. 메인 스레드도 동일한 뮤텍스로 모든 Simulator 접근을 직렬화.

**산출물**
- `src/app/main.cpp` — `tick_thread` 람다, `std::mutex sim_mutex`, `std::atomic<int> tick_ms`
- `speed <ms>` 명령으로 스레드 재시작 없이 틱 간격 원자적 변경

**트레이드오프**
사용자 명령이 최대 한 틱 간격 블로킹. 기본 300ms에서 체감 없음.

---

## AD-10: 그리드 기반 먼지 배치, 자동 감지, 소비

**배경**
기존 `injectDust(true)`는 수동 타이밍 필요하고 시각적 피드백 없었음. 장애물은 영구 그리드 모델 사용 — 먼지도 일관성을 위해 동일하게 적용.

**결정**
`Simulator`에 `_dust_cells` 추가. `tick()`에서 로봇 위치가 `_dust_cells`에 있으면 먼지 센서에 `true` 주입 후 셀 제거. `GridDisplay`는 `dustCells()`를 받아 청록색 `*`로 표시.

**산출물**
- `src/simulator/Simulator.hpp/cpp` — `placeDust(x,y)`, `dustCells()`, `tick()` 자동 감지
- `src/ui/GridDisplay.hpp/cpp` — `dust_cells` 파라미터, 청록색 `*` 렌더링
- `src/app/main.cpp` — `dust x y` 명령

**트레이드오프**
정밀 틱 제어 테스트를 위해 `injectDust(bool)` 유지.
