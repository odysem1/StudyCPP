# Changelog

## [v0.1] / 2026-04-14

### Added: Macros

- CS:APP 제공 기본 매크로 타이핑

## [v0.2] / 2026-04-15


### CS:APP 참고 및 기본 Implict free list(first-fit) 구현

### Added: extend_heap, coalesce, find_fit, place

- `extend_heap`: mem_sbrk를 호출해 heap을 늘림

- `coalesce`: 가용 블럭 생성 시 인접 블록의 allocated bit를 확인해 free block 병합(4가지 케이스)

- `find_fit`: 가용 블럭 탐색 알고리즘 구현(first-fit)

- `place`: 블럭을 나누어 필요한 만큼만 메모리를 Allocate하는 효율적인 Allocation function 구현

### Changed: mm_init, mm_free, mm_malloc

- `mm_init`: Prologue/Epilogue 블럭을 생성해 empty heap를 initialize하도록 변경

- `mm_free`: bp에 있는 블록을 free한 후 coalesce 함수 호출

- `mm_malloc`: naive 코드 제거, find_fit 및 place 함수 호출

- `mm_realloc`: naive 코드 제거, find_fit 및 place, free 함수 호출


## [v0.3] / 2026-04-17

### Explicit free list(first-fit) 구현

### Added: free_listp, MACROs ,putFreeBlock, removeFreeBLock

- `free_listp`: Free list의 맨 앞에 있는 블럭을 가리키는 포인터

- `MACROs`: Doubly linked list를 구현하기 위해 앞/뒤 가용 블럭을 다루는 매크로들 추가

- `putFreeBlock/removeFreeBlock`: 매크로를 이용해 Free block list에 새로운 블럭을 추가/제거하는 함수 추가

### Changed: coalesce, place, WSIZE&DSIZE

-`WSIZE & DSIZE`: 64비트 체제 적용을 위해 각각 8, 16으로 2배 늘림.

- `coalesce`: putFreeBlock/removeFreeBlock을 호출해 Free block list에 접근하는 기능 추가, 보기 편하도록 bp 변수 update 타이밍을 바꿔 그에 맞게 몇가지 코드 업데이트

- `place`: 메모리를 배정할 때 removeFreeBlock 호출로 Free block list 업데이트, 블럭을 나눠서 배정했을 때 남은 부분은 도로 putFreeBlock을 호출해 다시 집어넣음

## [v0.4] / 2026-04-18

### Segregated free list(first-fit) 구현

### Added: GET/SET_ADDR, get_index, insert/removeNode

- `GET/SET_ADDR`: 64비트 환경에서는 PUT으로 그대로 변수를 넣으면 void *와 int의 크기 차이로 오류가 발생할 수 있으므로 지정한 주소에 void * 값을 넣는 매크로를 따로 만듬

- `get_index`: 블럭의 크기를 입력으로 받아 어떤 크기의 List부터 탐색할지 index를 반환함(malloc lab trace file에 최적화되도록 그냥 if/else로 구현)

- `insert/removeNode`: Segregated free list에 가용 블럭을 넣고 빼는 함수를 추가함.

### Changed: find_fit, mm_init

- `find_fit`: 블럭의 크기에 맞춰 적당한 index의 free list부터 탐색하며 시간을 절약하도록 수정

- `mm_init`: Heap 맨 앞 LIST_LIMIT만큼의 칸을 segregated list에게 배정하고 그 뒤에 Heap을 initialize하도록 변경

### Removed: putFreeBlock, removeFreeBlock

- Segregated free list로 strategy를 변경했으므로 삭제

## [v0.4.1] / 2026-04-18

### Realloc 수정

### Added: get_size

- size를 입력받아 최소 블럭 크기인 16비트를 고려해 allocate할 size를 계산해주는 함수 추가

### Changed: mm_realloc

- free block을 탐색하기 전에 바로 뒷 블록을 살펴보는 매커니즘과 뒷 블럭이 없으면 heap을 extend시키는 부분을 추가해 Throughput을 향상시킴

## [v0.5] / 2026-04-25

### Realloc 수정 및 Best-fit 구현

### Added: split_block

- 블럭 시작 위치와 가용 크기, 필요 크기를 입력받아 블럭을 나눠서 allocate하고 남은 부분은 free한 상태로 만들어주는 함수

-place와 mm_realloc에서 중복된 알고리즘을 편리하게 사용하기 위해 구현

### Changed: find_fit, mm_realloc, place

- `find_fit`: best_fit 알고리즘 구현(Segregrated List 탐색)

- `mm_realloc`: 다음 블럭, 이전 블럭을 전부 탐색하고 allocate 하도록 알고리즘 수정 및 end of heap일 때 최적화, split_block 적용

- `place`: split_block 함수 적용

## [v0.6] / 2026-04-25

### Boundary tag optimization 구현, CHUNKSIZE 수정

### Added: PREV_ALLOC MARCROS

- 비가용 블럭의 Footer를 없애는 대신 헤더에 이전 블럭 Allocation 정보를 표기하는 구조로 변경, 필요한 매크로 추가

### Changed: get_size, PACK, Heap management functions, mm_init, CHUNKSIZE

- `get_size`: 헤더 구조 변경으로 인한 블럭 최소 사이즈 변경

- `PACK, Heap_managemnet functions, mm_init`: 헤더 구조 변경으로 인한 PACK 매크로 변경, 그에 다른 코드 수정

- `CHUNKSIZE`: util 점수 상승을 위해 크기 하향 조정(1>>12 -> 1>>10)