# Failures and Resolutions — 2026-05-17

Construction 중 발생한 장애와 그 근본 원인 및 해결 방법.

---

## F-01: CMake GTest 설정 실패

### 무엇이 실패했나
`src/CMakeLists.txt`가 `find_package(GTest REQUIRED)`로 작성되었다. CMake configure 시점에 빌드가 실패했다:

```
CMake Error: Could NOT find GTest
  (missing: GTEST_LIBRARY GTEST_INCLUDE_DIR GTEST_MAIN_LIBRARY)
```

### 근본 원인
시스템에 `libgtest-dev`가 설치되지 않았다. `find_package(GTest REQUIRED)`는 패키지가 없을 때 폴백 없이 치명적 오류로 처리한다.

### 왜 놓쳤나
`AGENTS.md`의 개발 및 CI 툴체인에 Google Test가 나열되어 있으므로 Ubuntu 24.04 WSL2 환경에 사전 설치되어 있을 것이라고 가정했다. CMakeLists를 작성하기 전에 해당 가정을 검증하지 않았다.

### 해결 방법
`find_package(GTest REQUIRED)`를 CMake `FetchContent`로 교체했다. configure 시점에 Google Test를 자동으로 다운로드하고 빌드한다:

```cmake
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)
```

이를 통해 시스템 패키지 상태에 관계없이 빌드가 자급자족 가능하며, CI 환경에도 더 적합하다.

### 예방
테스트 프레임워크 의존성은 시스템 설치를 보장할 수 없는 경우 `find_package(REQUIRED)` 대신 `FetchContent`를 사용한다. 업그레이드가 의도적으로 이루어지도록 `CMakeLists.txt`에 GTest 버전(현재 v1.14.0)을 문서화한다.

---

## F-02: Domain 클래스의 Getter Anti-Pattern — RvcController::state()

### 무엇이 실패했나
`RvcController`에 내부 `_state` enum을 반환하는 `state()` getter가 추가되었다:

```cpp
RvcState state() const { return _state; }
```

이를 사용하여 테스트가 작성되었다:
```cpp
EXPECT_EQ(controller.state(), RvcState::CLEANING);
EXPECT_EQ(controller.state(), RvcState::INTENSIFYING);
```

### 근본 원인
State가 테스트, `Simulator`, `ConsoleDisplay` 세 곳에서 필요했고, 가장 저항이 적은 경로는 직접 노출하는 것이었다. 당시 결합의 결과를 고려하지 않았다.

### 왜 문제인가
- 테스트가 내부 `RvcState` enum에 결합된다. State machine을 리팩터링하면(state 이름 변경, 분리, 병합) 동작이 변하지 않더라도 `state()`를 assert하는 모든 테스트가 깨진다.
- `ConsoleDisplay`가 `RvcState`에 의존하여 UI 계층이 controller의 내부 구현 세부사항을 알게 된다.
- 테스트는 내부 구현 레이블이 아닌 관측 가능한 동작(출력)을 검증해야 한다는 원칙을 위반한다.

### 해결 방법
1. `RvcController.hpp`에서 `state()`를 완전히 제거했다.
2. 모든 state 확인 assert를 motor와 cleaner 출력 로그를 사용하도록 재작성했다:

| 교체된 assert | 동작 기반 assert |
|---|---|
| `state() == CLEANING` | `cleaner.last() == ON` 또는 `motor.last() == FORWARD` |
| `state() == INTENSIFYING` | `cleaner.last() == POWER_UP` |
| `state() == IDLE` | `motor.log.empty()` 및 `cleaner.log.empty()` |

3. `ConsoleDisplay::render(RvcState, Direction, CleanPower)`를 `render(Direction, CleanPower)`로 변경했다. UI는 이제 관측 가능한 액추에이터 출력만 표시한다.

### 예방
Domain 클래스의 public 인터페이스는 **명령**과 **동작에 대한 쿼리**를 노출해야 하며, raw 내부 state 필드를 노출해서는 안 된다. 호출자가 현재 state를 알아야 한다면 "표시를 위한 것인가, 동작을 구동하기 위한 것인가?"를 물어라. 표시를 위한 것이라면 관측 가능한 출력을 표시하라. 동작을 구동하기 위한 것이라면 해당 로직은 domain 클래스 내부에 있어야 한다.

---

## F-03: Simulator의 Setter 명명 — setState() / setX()

### 무엇이 실패했나
시뮬레이션 제어 메서드가 `SimulatedSensor`에는 `setState(bool)`, `Simulator`에는 `setFront/setLeft/setRight/setDust()`로 명명되었다:

```cpp
void setState(bool state) { _state = state; }   // SimulatedSensor
void setLeft(bool state);                         // Simulator
```

### 근본 원인
이름이 간결함을 위해 선택되었으며, 이름만으로는 도메인 setter anti-pattern과 구별할 수 없다는 점을 고려하지 않았다.

### 왜 문제인가
프로젝트는 내부 state를 노출하는 단순 get/set 함수에 대한 명시적 규칙을 갖고 있다. 프로덕션 클래스의 `set*` 메서드는 위험 신호다. 시뮬레이션 인프라가 동일한 명명 패턴을 사용하면, 독자는 `set*` 호출이 규칙을 위반하는 것인지 아니면 테스트 인프라에서 정당하게 동작하는 것인지 구별할 수 없다.

컨벤션이 무너진다: test double이 규칙이 금지하는 것과 동일하게 명명되면, 코드 리뷰로 "get/set 금지"를 강제할 수 없다.

### 해결 방법
모든 시뮬레이션 제어 진입점을 `inject*(reading)`으로 이름을 변경했다:

```cpp
void inject(bool reading);          // SimulatedSensor
void injectLeft(bool reading);      // Simulator
void injectRight(bool reading);
void injectDust(bool reading);
void injectFront(bool reading);
```

`inject`라는 단어는 명확하게 "실제 HW를 우회하여 test double에 인위적으로 reading을 주입한다"는 의미를 전달한다. 이는 도메인 변이와 구별되는 시뮬레이션 전용 개념이다.

### 예방
test double에 대한 명명 규칙을 처음부터 확립한다:
- 시뮬레이션 제어 (인위적 데이터 주입) → `inject*()`
- 시뮬레이션 관찰 (기록된 출력 읽기) → `last()`, `log()`
- 테스트 인프라에서 `get*()/set*()`를 사용하지 않는다 — domain 코드에서 금지된 것과 동일한 이름.
