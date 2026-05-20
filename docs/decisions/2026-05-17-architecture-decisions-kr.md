# Architecture Decisions — 2026-05-17

Iteration 1의 Inception, Elaboration, Construction 단계에서 내린 설계 결정 사항들.

---

## AD-01: Navigation 로직에 Strategy Pattern 적용

**배경**  
`RvcController`는 현재 센서 데이터를 바탕으로 이동 방향을 결정해야 한다. Supplementary Specification(SUPP-02)은 향후 ML 기반 navigation으로 교체할 수 있도록, navigation을 controller 수정 없이 교체 가능하게 요구한다.

**결정 근거**  
navigation 로직이 `RvcController` 내부에 있으면, 새로운 알고리즘을 추가할 때 이미 다른 책임(state machine, 생명주기, 센서 오케스트레이션)을 가진 클래스를 수정해야 한다. 이는 OCP와 SRP를 동시에 위반한다.

**결정**  
navigation을 `INavigationStrategy`로 추출하고, `DefaultNavigationStrategy`를 첫 번째 구현체로 둔다. Strategy를 `RvcController`의 constructor를 통해 주입한다.

**산출물**  
- `src/interfaces/INavigationStrategy.hpp` — 순수 인터페이스
- `src/domain/DefaultNavigationStrategy.hpp/cpp` — 규칙 기반 첫 번째 구현체
- `RvcController`는 `INavigationStrategy*`를 보유하고, 구체 타입을 알지 못한 채 `navigate(SensorData)`를 호출

**Trade-off**  
navigation 변형마다 간접 호출 계층과 클래스가 하나씩 추가된다. navigation 교체 요구사항이 명시적이므로 수용한다.

---

## AD-02: FrontSensor는 Interrupt 방식, 나머지 센서는 Polling 방식

**배경**  
센서가 4개 존재한다. 예비 요구사항에 따르면 전방 장애물 감지 시 즉각 정지해야 하므로, 다음 주기적 tick까지 기다릴 수 없다.

**결정 근거**  
전방 센서를 매 tick마다 polling하면 tick 간격에 비례한 지연이 발생한다. Interrupt는 장애물이 감지되는 즉시 발생한다. 나머지 센서(left, right, dust)는 요구사항 테이블에서 명시적으로 주기적(periodic)으로 정의된다.

**결정**  
`FrontSensor`는 `onInterrupt()`(ISR에서 호출)를 제공하며, 이를 통해 `RvcController::onFrontObstacleDetected()`가 트리거된다. Left, Right, Dust 센서는 `detect()`만 구현하고 `onTick()` 내에서 polling된다.

**산출물**  
- `FrontSensor`에 두 개의 진입점: `detect()`(테스트용)와 `onInterrupt()`(실제 ISR용)
- `RvcController`에 두 개의 public 이벤트 핸들러: `onTick()`과 `onFrontObstacleDetected()`
- 설계가 비동기(interrupt)와 동기(tick) 제어 경로를 분리

**Trade-off**  
유지해야 할 코드 경로가 두 개로 늘어난다. HW 계약상 본질적 차이이므로 수용한다.

---

## AD-03: RvcController에 명시적 State Machine 도입

**배경**  
`RvcController`의 동작은 운영 모드(idle, cleaning, intensifying cleaning, 장애물 회피, 포위 탈출)에 따라 크게 달라진다.

**결정 근거**  
명시적 state machine이 없으면, 각 메서드(`onTick`, `onFrontObstacleDetected`)가 여러 플래그를 검사하는 중첩된 조건문을 포함하게 된다. 이러면 전이(transition)가 보이지 않고 새로운 state 추가가 위험해진다.

**결정**  
`RvcState` enum(IDLE, CLEANING, AVOIDING_OBSTACLE, ESCAPING, INTENSIFYING)을 도입한다. 모든 전이는 `RvcController`에서 명시적으로 이루어진다. AVOIDING_OBSTACLE과 ESCAPING은 일시적 상태로, `onFrontObstacleDetected()` 내에서 원자적으로 처리된 뒤 메서드 종료 전에 CLEANING으로 돌아온다.

**산출물**  
- `src/domain/RvcState.hpp`
- 모든 전이는 `RvcController.cpp`에서 직접 `_state =` 대입으로 가시화

**Trade-off**  
State enum은 내부 구현 세부사항이다. 외부에 노출하면(예: getter) 호출자와 결합이 생긴다 — 처리 방법은 AD-05 참조.

---

## AD-04: 모든 HW 의존성에 Constructor Injection 적용

**배경**  
`RvcController`는 4개의 센서, motor, cleaner, navigation strategy를 필요로 한다. 이것들은 실제 환경에서는 실제 HAL 객체이고, 테스트 환경에서는 시뮬레이션 객체가 된다.

**결정 근거**  
`RvcController`가 자체적으로 의존성을 생성하면(예: `new FrontSensor()`), 테스트에서 mock으로 교체할 수 없다. 클래스는 독립적으로 테스트할 수 없게 되고 DIP를 위반한다.

**결정**  
7개의 의존성을 모두 raw non-owning pointer로 constructor를 통해 주입한다. 조합 루트(`main.cpp` 또는 `Simulator`)가 객체를 소유하고 생명주기를 관리한다.

**산출물**  
- `RvcController` constructor가 7개의 인터페이스 포인터를 받음
- `Simulator`가 모든 구체 객체를 소유하고 주소를 전달
- `RvcControllerTest`는 `MockSensor`, `MockMotor`, `MockCleaner`를 스택에 생성하고 주소를 전달

**Trade-off**  
Raw pointer는 호출자가 생명주기를 보장해야 한다. 소유권이 명시적인 임베디드 스타일 C++이고, HAL 계층에서 `unique_ptr`는 이점 없이 오버헤드만 추가하므로 수용한다.

---

## AD-05: Domain 클래스에 state() Getter 금지 — 동작 기반 테스팅

**배경**  
초기 구현에서 테스트와 UI가 `_state`를 읽을 수 있도록 `RvcController::state()`를 추가했다. 코드 리뷰에서 유지보수 위험으로 지적되었다.

**결정 근거**  
getter로 내부 state를 노출하면 호출자가 내부 enum에 결합된다. State machine을 리팩터링하면(state 이름 변경, 분리, 병합), state()를 호출하는 모든 곳이 깨진다. `state() == CLEANING`을 확인하는 테스트는 동작이 아닌 구현을 테스트하는 것이다. TDD는 *시스템이 무엇을 하는가*를 검증해야지, *내부 모드를 무엇이라 부르는가*를 검증해서는 안 된다.

**결정**  
`RvcController`에서 `state()`를 제거한다. 테스트는 motor와 cleaner 명령 로그만을 통해 동작을 검증한다. UI(`ConsoleDisplay`)는 내부 state 레이블이 아닌 관측 가능한 출력(방향과 전력)을 표시한다.

**산출물**  
- `state()`를 `RvcController.hpp`에서 완전히 제거
- `ConsoleDisplay::render(Direction, CleanPower)` — `RvcState` 파라미터 없음
- `RvcControllerTest`와 `SimulatorTest`는 `motor.log`, `cleaner.log`, `lastDirection()`, `lastPower()`에 대해 assert

**Trade-off**  
런타임에 가시적인 state 레이블이 없어 디버깅이 약간 어려워진다. 동작 출력(방향 + 전력)으로 state를 충분히 추론할 수 있고 테스트가 더 견고해지므로 수용한다.

---

## AD-06: 시뮬레이션 전용 명명 규칙 — inject*() vs set*()

**배경**  
`SimulatedSensor`에 `setState(bool)`이, `Simulator`에 `setFront/setLeft/setRight/setDust()`가 있었다. 이것들은 도메인 setter anti-pattern과 외형이 동일하다.

**결정 근거**  
`sim.setLeft(true)`(시뮬레이션 제어)와 가상의 `rvc.setState(RvcState::IDLE)`(도메인 변이)를 이름만으로는 구별할 수 없다. 명명 규칙이 의도를 신호하지 못하면 "도메인 객체에 get/set 금지" 규칙이 약화된다.

**결정**  
모든 시뮬레이션 제어 메서드를 `inject*(reading)`으로 이름을 변경하여 시뮬레이션 전용 목적을 이름에서 명시한다. `inject`는 "test double에 인위적인 reading을 주입한다"는 의미다.

**산출물**  
- `SimulatedSensor::inject(bool reading)`
- `Simulator::injectFront/injectLeft/injectRight/injectDust(bool reading)`
- 테스트와 `main.cpp`의 모든 호출부 업데이트

**Trade-off**  
이름이 약간 길어진다. 테스트 인프라에서는 간결성보다 의도 명확성이 중요하므로 수용한다.
