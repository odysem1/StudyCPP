# Custom C++ Memory Allocation(Malloc Lab C++ Refactoring)

## Project Summary

군 복무 중 C++ 및 기초 이론을 복습하며 진행한 프로젝트입니다.

과거 학습했던 Malloc Lab을 처음부터 다시 구현한 이후, **Modern C++ 문법**을 이용해 리팩토링했습니다.
C++를 이용해 만든 **Custom Memory Pool** 의 로우 레벨 동작을 이해하고 구현하는 것이 목표입니다.

**개발 기간:** 2026.03 ~ 2026.05

---

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

---

## 구현 단계

### 1. Implicit/Explicit Free List
* C로 기초 알고리즘을 전부 세워 놓은 다음 C++로 전환
* First-Fit/Best-Fit, Coalescing 기초 논리 C로 완성

### 2. C++ Refactoring
* 전역 변수로 관리되던 Heap 포인터와 함수를 class 내부로 숨김
* void* 캐스팅 대신 *reinterpret_cast*를 적용해 타입 안전성을 높임

### 3. new/delete 오버로딩
* 사용자가 'new' 키워드로 객체를 생성할 때 OS 내부가 아니라 새 클래스에서 메모리를 떼어주도록 전역/클래스 오버로딩을 적용함

---

## TroubleShooting

### Issue 1

* Problem:
* Solve:
* Learned:

---

## Test result







