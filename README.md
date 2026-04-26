# Custom Memory Allocation(Malloc Lab Review)

## Project Summary

군 복무 중 메모리 할당 기초 이론을 복습하며 진행한 프로젝트입니다.

과거 학습했던 Malloc Lab을 그보다 발전시킨 알고리즘을 반영해 새로 구현했습니다.
**Memory Pool** 의 로우 레벨 동작을 이해하고 구현하는 것이 목표입니다.

**개발 기간:** 2026.04 ~ 2026.05

**Built with:** C


## 구현 단계

### 1. Implicit/Explicit Free List
* C로 과거 구현했던 기초 알고리즘을 전부 세워 놓기
* First-Fit/Best-Fit, Coalescing 기초 논리 C로 완성

### 2. Segregated Free List, Boundary tag optimization 구현
* 더 발전된 알고리즘인 Segregated Free List 적용, 크기별로 블럭 구분
* Boundary tag optimization으로 내부 단편화 최소화

### 3. Singly linked list 구조 구현


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

### Issue 5: Realloc 문제 - (v0.5)

* `Problem`: Realloc 부분에서 계속해서 util 점수 하락 발생
* `Solve`: 이전 블록을 보지 않고 계속해서 다음 블럭만 보니, 이전 블록이 free한 상태일 때를 고려하지 못해 계속해서 external fragmentation 발생
* `Learned`: If/else로 케이스를 나누어 구현할 때 더욱 세심한 케이스 구분이 필요함

### Issue 6: Singly linked list ghost footer - (v0.6)

* `Problem`: 내부 단편화 해결을 위해 최소 할당 크기를 16바이트로 만들고 Singly linked list로 관리하도록 만들었는데, split_block 과정에서 16바이트 블록이 생성될 때 다음 블록의 PREV_ALLOC을 갱신하지 않아, extend_heap 호출 시 coalesce가 시도되고 존재하지 않는 푸터를 읽어 Heap의 잘못된 공간에 접근
* `Solve`: 16바이트 블록은 Coalesce가 불가능하므로, 다음 블록의 PREV_ALLOC을 갱신하도록 수정
* `Learned`: 미니 블록 구현 시 다른 블록들에게 완전히 할당된 블럭처럼 보이게 만들어야 하고, 그러지 않으면 Heap corruption으로 이어질 수 있다는 것을 깨달음.

### Issue 7: External fragmentation - (v0.6)

* `Problem`: Realloc에서 External fragmentation 발생
* `Solve`: 128바이트 미만으로 남았을 시 블럭 분할 미실시로 의도적인 Internal fragmentation 발생시키기
* `Learned`: External과 Internal fragmentation이 Trade-off 관계에 있음을 실감함

## Test result

| Version | Strategy | Util | Thru | Total |
| :--- | :--- | :---: | :---: | :---: |
| v0.2 | Implicit List (First-fit) | 44 | 16 | 61 |
| v0.3 | Explicit List (First-fit) | 42 | 40 | 82 |
| v0.4 | Segragated list (First-fit) | 44 | 40 | 84 |
| v0.4.1 | Segragated list (In-place Realloc & split) | 44 | 40 | 84 |
| v0.5 | Segragated list (New Realloc & best-fit) | 46 | 40 | 84 |
| v0.6 | Segragated list (Boundary tag optimization) | 47 | 40 | 84 |
| v0.7 | Segragated list (Singly linked list, 128-split) | 52 | 40 | 92 |



