# Custom C++ Memory Allocation(Malloc Lab C++ Refactoring)

## Project Summary

군 복무 중 C++ 및 기초 이론을 복습하며 진행한 프로젝트입니다.

과거 학습했던 Malloc Lab을 처음부터 다시 구현한 이후, **Modern C++ 문법**을 이용해 리팩토링했습니다.
C++를 이용해 만든 **Custom Memory Pool** 의 로우 레벨 동작을 이해하고 구현하는 것이 목표입니다.

**개발 기간:** 2026.04 ~ 2026.05

**Built with:** C++23, C



## Why C++?

### Technically

Modern C++에서는 메모리 누수를 막기 위해 **스마트 포인터**를 이미 사용중이지만 객체의 **수명**을 안전하게 관리할 뿐, 메모리 공간 확보의 효율성은 결국 OS의 기본 범용 할당기와 같습니다.

잦은 객체 생성 및 소멸은 다음과 같은 문제를 불러일으킵니다.

1. **System Call Overhead:** 많은 동적 할당으로 인한 OS Lock 및 속도 저하
2. **Memory Fragmentation:** 힙 메모리 단편화
3. **Cache Miss:** CPU Cache Locality 하락

따라서 'new/delete' 를 오버로딩해서 OS의 느린 할당 메커니즘 대신 제가 만든 **Custom Memory Pool**에서 메모리 공간을 가져오도록 했습니다.

이 프로젝트의 최종 목표는 메모리 할당기의 **안전성**과 **속도 및 단편화 방어** 능력을 향상시키는 것입니다.

### Personally

언리얼 엔진에서 사용하는 언어인 C++과 그를 이용한 메모리 관리의 원리를 이해하고 객체지향 설계에 익숙해지기 위해서 만들었습니다.



## 구현 단계

### 1. Implicit/Explicit Free List
* C로 기초 알고리즘을 전부 세워 놓은 다음 C++로 전환
* First-Fit/Best-Fit, Coalescing 기초 논리 C로 완성

### 2. C++ Refactoring
* 전역 변수로 관리되던 Heap 포인터와 함수를 class 내부로 숨김
* void* 캐스팅 대신 *reinterpret_cast*를 적용해 타입 안전성을 높임

### 3. new/delete 오버로딩
* 사용자가 'new' 키워드로 객체를 생성할 때 OS 내부가 아니라 새 클래스에서 메모리를 떼어주도록 전역/클래스 오버로딩을 적용함



## TroubleShooting

### Issue 1: Bitmask error(v0.2)

* `Problem`: 모든 Testcase에서 Segment error 발생.
* `Solve`: GDB를 활용해 디버깅 후, 처음 init부터 Segment error가 발생한다는 것을 파악함, GET_SIZE 매크로에서 ~0x7을 -0x7이라고 타이핑한 오타를 찾고 수정함.
* `Learned`: Segment error 디버깅 시에 기초적인 GDB 활용법을 연습할 수 있었음.

### Issue 2: 포인터 크기 문제 - 64비트(v0.3)

* `Problem`: mm_init 단계부터 Segmentation error 발생
* `Solve`: 32비트 체제가 아닌 64비트 체제를 사용하다 보니, Implict list때는 괜찮았지만 Explicit list에서는 void\*\*를 사용하다 보니 void\*의 크기가 4비트가 아닌 8비트라 문제가 발생해서 WSIZE, DSIZE를 각각 8, 16비트로 수정함.
* `Learned`: 64비트와 32비트 체제에서 코딩할 때 low-level에서 메모리를 다룰 때 타입별로 크기가 다를 수 있으므로 주의해야 한다.

### Issue 3: 포인터 업데이트 순서 오류(v0.3)

* `Problem`: coalesce를 활용하는 특정 Testcase에서 Segment error 발생.
* `Solve`: GDB 디버깅 후 coalesce의 4가지 케이스 중 case 3와 4에서 잘못된 PUT 호출을 발견, Explicit free list를 구현하며 bp 포인터 값을 업데이트하는 시점을 바꿨는데 그것 때문이라고 판단해 PUT에 들어가는 변수값 수정
* `Learned`: 코드를 변경하면 늘 그것에 영향받을 수 있는 다른 부분을 생각하며 코딩하기.

### Issue 4: 포인터 크기 문제 2 - (v0.4)

* `Problem`: mm_init 단계부터 Segmentation error 발생
* `Solve`: 32비트 체제가 아닌 64비트 체제를 사용하다 보니, Implict list때는 괜찮았지만 Explicit list에서는 void\*\*를 사용하다 보니 void\*의 크기가 4비트가 아닌 8비트라 문제가 발생해서 8비트 데이터를 불러오고 업데이트하기 위한 GET_ADDR랑 PUT_ADDR 매크로를 따로 만듬
* `Learned`: 64비트와 32비트 체제의 차이를 주의해야 한다는 점을 다시 상기함

## Test result

| Version | Strategy | Util | Thru | Total |
| :--- | :--- | :---: | :---: | :---: |
| v0.2 | Implicit List (First-fit) | 44 | 16 | 61 |
| v0.3 | Explicit List (First-fit) | 42 | 40 | 82 |
| v0.4 | Segragated list (First-fit) | 44 | 40 | 84 |




