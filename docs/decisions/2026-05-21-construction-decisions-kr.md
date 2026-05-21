# 아키텍처 결정 사항 — 2026-05-21 (Control SW)

Construction 단계 2차 이터레이션에서 RVC Control SW에 영향을 미치는 결정.

---

## AD-07: onTick()은 매 사이클마다 FORWARD를 재발행해야 한다

**배경**
`start()` 호출 이후 RVC는 장애물이 감지되거나 `stop()`이 호출될 때까지 계속 전진해야 한다. 초기 구현은 `start()` 안에서 `FORWARD`를 한 번만 발행했다.

**결정 근거**
실제 H-브리지 하드웨어는 다시 명령을 받을 때까지 방향을 래치(latch)하므로, 실제 제품에서는 시작 시 한 번의 `FORWARD`로 충분하다. 그러나 `IMotorController` 인터페이스 계약은 이런 보장을 하지 않는다 — 각 `move()` 호출은 독립적인 명령이며, 호출자는 틱 경계 간 연속성을 가정할 수 없다. Control SW는 구현 세부사항에 무관하게 매 틱마다 전진 의도를 명시적으로 표현해야 한다.

**결정**
`onTick()`은 CLEANING 또는 INTENSIFYING 상태일 때 매 틱 마지막에 `_motor->move(Direction::FORWARD)`를 발행한다.

**산출물**
- `src/app/RvcController.cpp` — `onTick()`에서 `_state == CLEANING || INTENSIFYING` 조건으로 FORWARD 추가

**트레이드오프**
실제 하드웨어에는 매 틱마다 중복 명령이 전송된다(하드웨어는 이미 방향이 래치되어 있으므로 무시). `IMotorController` 구현 세부사항에 대한 가정 없이 인터페이스 계약을 자급자족(self-contained)하게 만들기 위해 허용한다.
