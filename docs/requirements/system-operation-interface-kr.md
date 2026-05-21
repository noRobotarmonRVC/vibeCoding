# System Operation Interface (한국어)

| «interface» RVCSystem | `startCleaning()` · `tick()` · `onFrontObstacleDetected()` · `stopCleaning()` |
|---|---|

System Operation은 Use-Case 시나리오의 시스템 이벤트에서 도출된다. 각 오퍼레이션은 외부 액터가 RVC 시스템에 보내는 메시지를 나타낸다.

---

## 1. 오퍼레이션 요약

| 오퍼레이션 | 트리거 액터 | 관련 UC |
|---|---|---|
| `startCleaning()` | User | UC-01 |
| `tick()` | Timer | UC-02, UC-03, UC-04, UC-05 |
| `onFrontObstacleDetected()` | Front Sensor | UC-03, UC-04 |
| `stopCleaning()` | User | UC-06 |

---

## 2. 시스템 오퍼레이션 명세

---

### SO-01: `startCleaning()`

| 항목 | 내용 |
|---|---|
| **오퍼레이션** | `startCleaning()` |
| **관련 UC** | UC-01: 청소 세션 시작 |
| **트리거 액터** | User |
| **설명** | RVC를 IDLE 상태에서 CLEANING 상태로 전환하고, Cleaner를 활성화하며, 전진을 시작한다. |
| **사전 조건** | RVC가 IDLE 상태다. |
| **사후 조건** | RVC 상태 = CLEANING. Motor 명령 = FORWARD. Cleaner 명령 = ON. |

**참조:** UC-01 Step 1–4

---

### SO-02: `tick()`

| 항목 | 내용 |
|---|---|
| **오퍼레이션** | `tick()` |
| **관련 UC** | UC-02, UC-03, UC-04, UC-05 |
| **트리거 액터** | Timer |
| **설명** | RVC가 활성 상태일 때 모든 센서 폴링과 navigation 결정을 구동하는 주기적 신호. 매 tick마다 Left, Right, Dust 센서를 읽고 다음 동작을 결정한다. |
| **사전 조건** | RVC가 활성 상태다 (CLEANING, AVOIDING_OBSTACLE, ESCAPING, INTENSIFYING 중 하나). |
| **사후 조건** | 현재 센서 값에 따라 Motor 방향과 Cleaner 상태가 업데이트된다. 아래 표에 따라 상태가 전이될 수 있다. |

**`tick()` 에 의한 상태 전이:**

| 센서 읽기 | 전이 상태 | Motor | Cleaner |
|---|---|---|---|
| 장애물 없음, 먼지 없음 | CLEANING | FORWARD | ON |
| Front = True, Left 또는 Right 개방 | AVOIDING_OBSTACLE | STOP → TURN → FORWARD | ON |
| Front = True, Left = True, Right = True | ESCAPING | BACKWARD → TURN → FORWARD | ON |
| Dust = True | INTENSIFYING | FORWARD | POWER_UP |
| 강화 지속 시간 경과 | CLEANING | FORWARD | ON |

**참조:** UC-02 주요 성공 시나리오 및 대안 흐름

---

### SO-03: `onFrontObstacleDetected()`

| 항목 | 내용 |
|---|---|
| **오퍼레이션** | `onFrontObstacleDetected()` |
| **관련 UC** | UC-03, UC-04 |
| **트리거 액터** | Front Sensor (interrupt 방식) |
| **설명** | 전방 장애물 감지 시 발생하는 interrupt 기반 알림. `tick()`과 달리 비동기적으로, 폴링 주기가 아닌 감지 즉시 발생한다. |
| **사전 조건** | RVC가 CLEANING 또는 AVOIDING_OBSTACLE 상태다. |
| **사후 조건** | Motor 명령 = STOP. 시스템이 Left/Right 센서를 읽고 다음 상태(AVOIDING_OBSTACLE 또는 ESCAPING)를 결정한다. |

**결정 로직:**

| Left Sensor | Right Sensor | 전이 상태 |
|---|---|---|
| False (개방) | 무관 | AVOIDING_OBSTACLE — 좌회전 |
| 무관 | False (개방) | AVOIDING_OBSTACLE — 우회전 |
| True | True | ESCAPING |

**참조:** UC-03 Step 1–3, UC-03 대안 흐름, UC-04 Step 1

---

### SO-04: `stopCleaning()`

| 항목 | 내용 |
|---|---|
| **오퍼레이션** | `stopCleaning()` |
| **관련 UC** | UC-06: 청소 세션 종료 |
| **트리거 액터** | User |
| **설명** | 활성 청소 세션을 종료하고, Motor를 정지하며, Cleaner를 끈다. |
| **사전 조건** | RVC가 활성 상태다 (CLEANING, AVOIDING_OBSTACLE, ESCAPING, INTENSIFYING 중 하나). |
| **사후 조건** | RVC 상태 = IDLE. Motor 명령 = STOP. Cleaner 명령 = OFF. |

**참조:** UC-06 Step 1–4

---

## 3. 시스템 시퀀스 다이어그램

### 시나리오 A: 정상 청소 (장애물 없음, 먼지 없음)

```
User          RVC System       Timer
 |                |               |
 |--startCleaning()-->            |
 |                |               |
 |                |<----tick()----|
 |                |  [장애물 없음, 먼지 없음]
 |                |  Motor: FORWARD, Cleaner: ON
 |                |               |
 |                |<----tick()----|
 |                |  (반복)        |
 |                |               |
 |--stopCleaning()-->             |
 |                |               |
```

### 시나리오 B: 전방 장애물 회피

```
User       Front Sensor     RVC System       Timer
 |               |               |               |
 |--startCleaning()------------>|               |
 |               |               |<----tick()----|
 |               |               |  Motor: FORWARD
 |               |               |               |
 |               |--onFrontObstacleDetected()--->|
 |               |               |  Motor: STOP
 |               |               |  [Left/Right 읽기]
 |               |               |  Motor: TURN → FORWARD
 |               |               |               |
 |               |               |<----tick()----|
 |               |               |  (navigation 재개)
```

### 시나리오 C: 청소 강도 높이기

```
User          RVC System       Timer        Dust Sensor
 |                |               |               |
 |--startCleaning()-->            |               |
 |                |<----tick()----|               |
 |                |  [Dust Sensor = True]         |
 |                |  Cleaner: POWER_UP            |
 |                |               |               |
 |                |<----tick()----|               |
 |                |  [지속 시간 경과]              |
 |                |  Cleaner: ON (정상)           |
```
