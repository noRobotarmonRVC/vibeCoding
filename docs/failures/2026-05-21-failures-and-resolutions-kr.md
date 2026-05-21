# 실패 및 해결 — 2026-05-21 (Control SW)

Construction 단계 2차 이터레이션에서 RVC Control SW에서 발생한 실패.

---

## F-04: start() 이후 RVC가 움직이지 않음

### 무엇이 실패했는가
`start()` 호출 후 여러 틱을 진행해도 로봇이 초기 위치에서 움직이지 않았다. Tick 카운터만 올라갔다. 모터 로그에는 `start()`의 `FORWARD` 하나만 있고 그 이후에는 아무것도 없었다.

### 근본 원인
`RvcController::start()`가 `_motor->move(Direction::FORWARD)`를 한 번 발행했다. `RvcController::onTick()`은 CLEANING 상태에서 먼지 센서만 확인하고 모터 명령을 전혀 발행하지 않았다.

`IMotorController` 인터페이스는 방향이 틱 간에 유지된다는 보장을 하지 않는다. 각 `move()` 호출은 독립적인 명령이다. `start()`에서 발행된 FORWARD가 소비된 후 이후 틱에서는 모터 출력이 없었다.

### 왜 놓쳤는가
"FORWARD를 한 번 발행하면 모터가 계속 전진 상태를 유지한다"고 가정했다. 이는 실제 H-브리지 하드웨어에서는 맞는 말이다. Control SW가 인터페이스 계약 대신 하드웨어의 구현 세부사항에 의존한 것이 문제였다.

### 해결책
`onTick()`에서 `_state == CLEANING || _state == INTENSIFYING` 조건일 때 `_motor->move(Direction::FORWARD)` 추가:

```cpp
if (_state == RvcState::CLEANING || _state == RvcState::INTENSIFYING) {
    _motor->move(Direction::FORWARD);
}
```

### 예방책
`RvcController`는 `IMotorController`를 상태 없는(stateless) 명령 수신자로 취급해야 한다. 연속적인 행동은 매 틱마다 재명령해야 한다. 하드웨어 지식이 아닌 인터페이스 계약이 컨트롤러가 가정할 수 있는 것을 정의한다.
