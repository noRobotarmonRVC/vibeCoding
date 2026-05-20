# Design Model

Design Model은 RVC Control SW의 소프트웨어 구조 — 클래스, 인터페이스, 책임, 주요 협력 관계를 정의한다. 모든 명명은 프로젝트 코드 컨벤션을 따른다.

---

## 1. 적용된 Design Principles

- **SRP**: 각 클래스는 변경 이유가 하나다 (센서 읽기, navigation 로직, 액추에이터 제어, 오케스트레이션이 분리됨).
- **OCP**: 새로운 센서 타입과 navigation strategy는 기존 클래스 수정 없이 새 클래스 추가로 확장된다.
- **LSP**: 모든 `ISensor` 구현체는 `RvcController`에서 상호 교체 가능하다.
- **ISP**: `ISensor`, `IMotorController`, `ICleanerController`는 별도 인터페이스이며, 어떤 클래스도 관련 없는 메서드를 구현하도록 강제되지 않는다.
- **DIP**: `RvcController`는 구체 구현이 아닌 추상화(`ISensor`, `IMotorController`, `INavigationStrategy`)에 의존한다.

---

## 2. Enumerations

```cpp
enum class Direction  { FORWARD, BACKWARD, LEFT, RIGHT, STOP };
enum class CleanPower { OFF, ON, POWER_UP };

enum class RvcState {
    IDLE,
    CLEANING,           // 정상 전진 navigation
    AVOIDING_OBSTACLE,  // 전방 차단, 측면 중 하나 이상 개방
    ESCAPING,           // 포위 상태 (전방 + 좌측 + 우측 모두 차단)
    INTENSIFYING        // 먼지 감지, power up 활성
};
```

---

## 3. Interfaces

### `ISensor`
```
ISensor
─────────────────────────────
+ detect() : bool   {pure virtual}
```
구현체: `FrontSensor`, `LeftSensor`, `RightSensor`, `DustSensor`.

### `IMotorController`
```
IMotorController
─────────────────────────────
+ move(direction : Direction) : void   {pure virtual}
```

### `ICleanerController`
```
ICleanerController
─────────────────────────────
+ setPower(power : CleanPower) : void   {pure virtual}
```

### `INavigationStrategy`
```
INavigationStrategy
─────────────────────────────
+ navigate(data : SensorData) : Direction   {pure virtual}
```
navigation 알고리즘을 제어 오케스트레이션으로부터 분리한다. `RvcController`를 수정하지 않고도 규칙 기반 로직을 ML 기반 strategy로 교체할 수 있게 한다.

---

## 4. Value Object

### `SensorData`
단일 제어 사이클의 모든 센서 readings를 집계한다.

```
SensorData
─────────────────────────────
+ is_front_blocked : bool
+ is_left_blocked  : bool
+ is_right_blocked : bool
+ has_dust         : bool
```

---

## 5. Classes

### `FrontSensor : ISensor`
```
FrontSensor
─────────────────────────────
- _triggered : bool
─────────────────────────────
+ detect() : bool
+ onInterrupt() : void    ← interrupt 핸들러에서 호출
```

### `LeftSensor : ISensor`, `RightSensor : ISensor`, `DustSensor : ISensor`
```
[Sensor]
─────────────────────────────
- _state : bool
─────────────────────────────
+ detect() : bool
```

### `DefaultNavigationStrategy : INavigationStrategy`
요구사항에서 정의한 규칙 기반 navigation 로직을 구현한다.

```
DefaultNavigationStrategy
─────────────────────────────
+ navigate(data : SensorData) : Direction
```

인코딩된 규칙 (우선순위 순서):
1. 전방 + 좌측 + 우측 차단 → BACKWARD (호출자가 ESCAPING으로 전이 후 회전)
2. 전방 차단, 측면 개방 → LEFT 또는 RIGHT (개방된 측면으로 회전)
3. 장애물 없음 → FORWARD

### `RvcController`
중앙 오케스트레이터. 모든 컴포넌트 참조를 보유하고 RVC state machine을 관리한다.

```
RvcController
─────────────────────────────────────────────────────
- _front_sensor      : ISensor*
- _left_sensor       : ISensor*
- _right_sensor      : ISensor*
- _dust_sensor       : ISensor*
- _motor             : IMotorController*
- _cleaner           : ICleanerController*
- _nav_strategy      : INavigationStrategy*
- _state             : RvcState
- _intensify_ticks   : int       ← PowerUp 지속 시간 카운트다운
─────────────────────────────────────────────────────
+ start() : void
+ stop()  : void
+ onTick() : void                ← Timer tick마다 호출
+ onFrontObstacleDetected() : void   ← interrupt에서 호출
```

---

## 6. Class Diagram (개요)

```
          ┌─────────────────────────────────────────────────┐
          │                  RvcController                   │
          │  - _state : RvcState                            │
          └──┬────────┬────────┬────────┬────────┬─────────┘
             │        │        │        │        │
         ISensor  ISensor  ISensor  ISensor  IMotorController  ICleanerController  INavigationStrategy
             │        │        │        │
       FrontSensor  LeftSensor  RightSensor  DustSensor

    INavigationStrategy
          │
    DefaultNavigationStrategy
```

---

## 7. Sequence Diagrams

### UC-02: 정상 Navigation (장애물 없음, 먼지 없음)

```
Timer          RvcController        ISensor(x3+dust)    IMotorController   ICleanerController
  │                  │                    │                    │                   │
  │──onTick()───────>│                    │                    │                   │
  │                  │──detect()─────────>│ (left)             │                   │
  │                  │<──false────────────│                    │                   │
  │                  │──detect()─────────>│ (right)            │                   │
  │                  │<──false────────────│                    │                   │
  │                  │──detect()─────────>│ (dust)             │                   │
  │                  │<──false────────────│                    │                   │
  │                  │                    │                    │                   │
  │                  │──navigate(data)──> [DefaultNavigationStrategy]              │
  │                  │<──FORWARD──────────│                    │                   │
  │                  │                    │                    │                   │
  │                  │────────────────────────move(FORWARD)───>│                   │
  │                  │────────────────────────────────────────────setPower(ON)────>│
```

---

### UC-03: 전방 장애물 회피

```
FrontSensor    RvcController        ISensor(L/R)        IMotorController   ICleanerController
  │                  │                    │                    │                   │
  │──onFrontObstacleDetected()──────────>│                    │                   │
  │                  │────────────────────────move(STOP)──────>│                   │
  │                  │──detect()─────────>│ (left)             │                   │
  │                  │<──false────────────│                    │                   │
  │                  │   [left 개방 → LEFT로 회전]             │                   │
  │                  │────────────────────────move(LEFT)──────>│                   │
  │                  │────────────────────────move(FORWARD)───>│                   │
  │                  │   [state = CLEANING]                    │                   │
```

---

### UC-04: 포위 상태 탈출

```
FrontSensor    RvcController        ISensor(L/R)        IMotorController
  │                  │                    │                    │
  │──onFrontObstacleDetected()──────────>│                    │
  │                  │──detect()─────────>│ (left) → true      │
  │                  │──detect()─────────>│ (right) → true     │
  │                  │   [state = ESCAPING]                    │
  │                  │────────────────────────move(BACKWARD)──>│
  │                  │   [회전 방향 선택]                      │
  │                  │────────────────────────move(LEFT/RIGHT)>│
  │                  │────────────────────────move(FORWARD)───>│
  │                  │   [state = CLEANING]                    │
```

---

### UC-05: 청소 강도 높이기

```
Timer          RvcController        DustSensor          ICleanerController
  │                  │                    │                    │
  │──onTick()───────>│──detect()─────────>│                    │
  │                  │<──true─────────────│                    │
  │                  │   [state = INTENSIFYING]                │
  │                  │────────────────────────setPower(POWER_UP)>│
  │                  │   [_intensify_ticks = INTENSIFY_DURATION]│
  │                  │                    │                    │
  │   ... tick 경과 ...                   │                    │
  │                  │   [_intensify_ticks가 0에 도달]         │
  │                  │────────────────────────setPower(ON)─────>│
  │                  │   [state = CLEANING]                    │
```

---

## 8. State Machine: RvcController

```
         start()
  [IDLE] ──────────────────────────────────────────> [CLEANING]
                                                         │   ▲
                      onFrontObstacleDetected()           │   │ 회전 + 전진
            ┌─────── (측면 개방) ─────────────────> [AVOIDING_OBSTACLE]
            │                                             │
            │        onFrontObstacleDetected()            │
            └─────── (전방향 차단) ──────────────> [ESCAPING]
                                                         │   ▲
                                                         └───┘ 후진 + 회전 + 전진
            
  [CLEANING] ──── 먼지 감지 ──────────────────────> [INTENSIFYING]
  [INTENSIFYING] ─ 타이머 만료 ───────────────────> [CLEANING]

  모든 상태 ──── stop() ──────────────────────────> [IDLE]
```
