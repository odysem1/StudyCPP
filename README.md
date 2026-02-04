# StudyCPP

----

## socketTest - 숫자 야구

C++ 소켓 통신을 활용한 숫자 야구 게임 > 실시간 데이터 전송을 구현하기 위한 프로젝트입니다.

---

### 기능

**Socket Programming**: TCP/IP 소켓을 이용한 통신기능 구현

**1v1 Real-time**: 2명이 동시에 접속해서 실시간으로 결과를 주고받음

**Algorithm**: 게임 규칙 구현 및 예외 처리

---

### 구성요소

**언어**: C++

**Library**: sys/socket.h

**Protocol**: TCP

---

### How to Run

#### 1. Server 실행

클라이언트 접속 대기 및 흐름 제어

```Bash
g++ -o server socketTest/server/server.cpp
./server [Port Number]
```

#### 2. Client 실행

```Bash
g++ -o client socketTest/client/client.cpp
./client [Server IP] [Port Number]
```

---

### Game rule

통상적인 3글자 숫자 야구와 동일합니다!

---

### Architecture

```Plaintext
Client1 <---TCP---> Server <---TCP---> Client2
           Guess      ->  
                             Guess       ->
    <-     Result       
```

1. Client1에서 TCP로 서버에 추측 내용 전달
2. 서버에서 Client2에게 추측 내용 전달
3. 서버에서 Client1에게 결과 전달