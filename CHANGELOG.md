#Changelog
---
## [0.1] - 2026-04-14

### Added: Macros

- CS:APP 제공 기본 매크로 타이핑

## [0.2] - 2026-04-15


### CS:APP 제공 기본 틀 구현

### Added: extend_heap, coalesce

- `extend_heap`: mem_sbrk를 호출해 heap을 늘림

- `coalesce`: 가용 블럭 생성 시 인접 블록의 allocated bit를 확인해 free block 병합(4가지 케이스)

### Changed: mm_init, mm_free, mm_malloc

-`mm_init`: empty heap를 initialize하도록 변경

-`mm_free`: bp에 있는 블록을 free한 후 coalesce 함수 호출

-`mm_malloc`: naive 코드 제거, find_fit 및 place 함수 호출