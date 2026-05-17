# RVC Control SW

OOAD / Unified Process(UP) 방법론으로 개발한 로봇 청소기(RVC) 제어 소프트웨어입니다.

---

## 요구 환경

| 항목 | 버전 |
|---|---|
| OS | Ubuntu 24.04 (WSL2 포함) |
| 컴파일러 | GCC 또는 Clang (C++17 이상) |
| 빌드 시스템 | CMake 3.14 이상 |
| 테스트 프레임워크 | Google Test (CMake FetchContent로 자동 설치) |

---

## 빌드 및 실행

```bash
# 1. 빌드 디렉터리 생성 및 CMake 구성
cmake -S src -B build

# 2. 빌드
cmake --build build

# 3. 메인 프로그램 실행
./build/rvc_main

# 4. 전체 테스트 실행
cd build && ctest --output-on-failure

# 5. 특정 테스트 바이너리 직접 실행
./build/rvc_tests
```

> `build/` 디렉터리는 `.gitignore`에 포함되어 있으므로 커밋되지 않습니다.

---

## 디렉터리 구조

```
OOADProject/
├── src/
│   ├── interfaces/        # 인터페이스 헤더 (ISensor, IMotorController 등)
│   ├── domain/            # 비즈니스 로직 (DefaultNavigationStrategy, enum 정의)
│   ├── hal/               # 하드웨어 추상화 계층 (FrontSensor, DustSensor 등)
│   ├── app/               # 애플리케이션 계층 (RvcController, main.cpp)
│   ├── simulator/         # 시뮬레이터 (하드웨어를 소프트웨어로 대체)
│   └── ui/                # 콘솔 UI (ConsoleDisplay)
├── test/
│   ├── app/               # RvcController 단위 테스트
│   ├── domain/            # DefaultNavigationStrategy 단위 테스트
│   └── simulator/         # 통합 테스트
├── docs/
│   ├── requirements/      # 비전, 유스케이스 모델, 보충 명세, 용어집
│   └── design/            # 도메인 모델, 설계 모델, SW 아키텍처 문서
├── .gitignore
├── CLAUDE.md
└── README.md
```

---

## 코드 아키텍처

### 4계층 구조

```
┌──────────────────────────────────────────────┐
│             Application Layer                 │
│   RvcController  (상태 머신, 오케스트레이션)   │
├──────────────────────────────────────────────┤
│              Domain Layer                     │
│   DefaultNavigationStrategy  SensorData       │
│   RvcState · Direction · CleanPower (enum)    │
├──────────────────────────────────────────────┤
│             Interface Layer                   │
│   ISensor  IMotorController  ICleaner...      │
│   INavigationStrategy                         │
├──────────────────────────────────────────────┤
│      Hardware Abstraction Layer (HAL)         │
│   FrontSensor  LeftSensor  RightSensor        │
│   DustSensor                                  │
└──────────────────────────────────────────────┘
```

상위 계층은 하위 계층에 의존하고, 하위 계층은 절대 상위 계층을 참조하지 않습니다. 계층 간 통신은 모두 인터페이스를 통해 이루어집니다.

### 핵심 설계 결정

| 결정 | 내용 |
|---|---|
| AD-01: Strategy 패턴 | 내비게이션 로직을 `INavigationStrategy`로 분리해 ML 알고리즘 등으로 교체 가능 |
| AD-02: 인터럽트 vs 폴링 | `FrontSensor`는 인터럽트 방식(`onInterrupt()`), 나머지 센서는 매 Tick 폴링 |
| AD-03: 상태 머신 | `RvcController`가 `RvcState` enum으로 상태를 명시적으로 관리 |
| AD-04: 의존성 주입 | 모든 하드웨어 의존성을 생성자 주입으로 전달 → Google Test 목(Mock) 격리 가능 |

### 상태 머신 (`RvcState`)

```
IDLE ──start()──► CLEANING ──front obstacle──► AVOIDING_OBSTACLE
                     ▲                               │
                     │         left/right blocked    │
                     │              ▼                │
                  INTENSIFYING ◄── ESCAPING ◄────────┘
                     │
                  (dust 감지 후 일정 Tick 경과 시 CLEANING으로 복귀)
```

---

## 테스트 전략

- **단위 테스트**: Google Test + GMock으로 각 클래스를 독립적으로 검증합니다. 하드웨어 의존성은 Mock으로 대체합니다.
- **통합 테스트**: `Simulator`가 실제 하드웨어 대신 `IMotorController`, `ICleanerController`, `ISensor`를 구현하여 전체 제어 루프를 검증합니다.

---

## 네이밍 규칙

| 대상 | 스타일 | 예시 |
|---|---|---|
| 클래스 | PascalCase | `SensorController` |
| 인터페이스 | `I` 접두사 | `ISensor` |
| 추상 클래스 | `Abstract` 접두사 | `AbstractCleaner` |
| 메서드 | camelCase | `detectObstacle()` |
| public 변수 | snake_case | `front_sensor` |
| private 변수 | `_` 접두사 snake_case | `_speed` |
| 상수 | UPPER_SNAKE_CASE | `MAX_SPEED` |
| 디렉터리 | kebab-case | `sensor-drivers/` |

---

## CI 파이프라인

GitHub Actions에서 Push마다 자동 실행됩니다.

1. CMake 빌드
2. Google Test 실행
3. clang-tidy 정적 분석
4. gcovr 코드 커버리지 리포트 생성
