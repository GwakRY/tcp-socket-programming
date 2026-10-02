# TCP Socket Programming

컴퓨터 네트워크 전공 과목에서 진행한 C/Linux 기반 소켓 프로그래밍 프로젝트입니다.  
TCP/IP 기반 **Daytime Client/Server**와 **Echo Client/Server**를 구현하며 Client–Server 연결 수립, IPv4 주소 처리, 데이터 송수신 과정을 학습했습니다.

---

## Project Overview

- **Language**: C
- **Environment**: Linux
- **Protocol**: TCP/IP
- **Main Focus**: Socket Programming, Client–Server Communication, IPv4

본 프로젝트는 다음 두 가지 TCP 프로그램으로 구성됩니다.

1. **Daytime Client/Server**
2. **Echo Client/Server**

---

## 1. TCP Daytime Client / Server

### Daytime Client

서버의 IP 주소를 입력받아 TCP 연결을 수립하고, 서버가 전송한 문자열 데이터를 수신하는 Client 프로그램입니다.

#### 주요 기능

- `socket()`을 이용한 TCP socket 생성
- `sockaddr_in` 구조체를 이용한 서버 주소 설정
- `htons()`를 이용한 포트 번호 변환
- `inet_pton()`을 이용한 IPv4 문자열 주소 변환
- `connect()`를 이용한 TCP 서버 연결
- `read()`를 이용한 서버 데이터 수신
- 수신 문자열 출력

### Daytime Server

TCP 연결을 대기하고, Client가 접속하면 현재 시각 정보를 문자열로 전송하는 Server 프로그램입니다.

#### 주요 기능

- `socket()`을 이용한 listening socket 생성
- `INADDR_ANY`와 `htonl()`을 이용한 서버 주소 설정
- `bind()`를 이용한 IP/Port 결합
- `listen()`을 이용한 Client 연결 대기
- `accept()`를 이용한 연결 수립
- `time()` / `ctime()` 기반 현재 시각 문자열 생성
- `write()`를 이용한 Client 데이터 전송
- 통신 종료 후 연결 socket 정리

### Connection Flow

```text
Client                              Server
  |                                   |
  | socket()                          | socket()
  |                                   | bind()
  |                                   | listen()
  | connect() ----------------------> | accept()
  |                                   |
  | <--------- Time Data ------------ | write()
  | read()                            |
  |                                   |
```

---

## 2. TCP Echo Client / Server

### Echo Client

사용자가 입력한 문자열을 Server에 전송하고, Server가 반환한 동일한 데이터를 수신하여 출력합니다.

#### 주요 기능

- IP 주소와 Port를 입력받아 서버 연결
- `socket()` / `connect()` 기반 TCP Client 구현
- `fgets()`로 사용자 입력 처리
- `write()`를 이용한 데이터 전송
- `read()`를 반복 호출하여 전송한 데이터 길이만큼 응답 수신
- `q` / `Q` 입력 시 Client 종료
- 통신 종료 후 socket descriptor 정리

### Echo Server

Client의 연결 요청을 수락한 후 수신한 데이터를 그대로 다시 전송하는 Echo Server입니다.

#### 주요 기능

- `socket()` / `bind()` / `listen()` / `accept()` 기반 TCP Server 구성
- Client가 보낸 데이터를 `read()`로 수신
- 수신 데이터를 `write()`로 다시 Client에 전송
- 연결 종료 후 socket descriptor 정리
- 반복적인 Client 연결 요청 처리

### Echo Communication Flow

```text
Client                              Server
  |                                   |
  | socket()                          | socket()
  |                                   | bind()
  |                                   | listen()
  | connect() ----------------------> | accept()
  |                                   |
  | -------- "hello" --------------> | read()
  |                                   | write()
  | <------- "hello" ---------------- |
  | read()                            |
```

---

## Socket API Used

### Server Side

```text
socket()
   ↓
bind()
   ↓
listen()
   ↓
accept()
   ↓
read() / write()
   ↓
close()
```

### Client Side

```text
socket()
   ↓
inet_pton()
   ↓
connect()
   ↓
read() / write()
   ↓
close()
```

---

## IPv4 Address Handling

TCP 통신을 위해 `sockaddr_in` 구조체를 사용했습니다.

주요 요소:

- `AF_INET`: IPv4 사용
- `SOCK_STREAM`: TCP socket
- `sin_port`: 서버 Port
- `sin_addr`: IPv4 주소

주소 및 Port 처리에는 다음 함수를 사용했습니다.

- `htons()`  
  Host byte order의 Port 값을 Network byte order로 변환

- `htonl()`  
  Host byte order의 32-bit 값을 Network byte order로 변환

- `inet_pton()`  
  문자열 형태의 IPv4 주소를 Network address 구조로 변환

---

## Tech Stack

- **Language**: C
- **Environment**: Linux
- **Network**: TCP/IP, IPv4
- **API**:
  - `socket()`
  - `bind()`
  - `listen()`
  - `accept()`
  - `connect()`
  - `read()`
  - `write()`
  - `close()`
  - `htons()`
  - `htonl()`
  - `inet_pton()`

---

## Build

프로젝트에서 사용하는 `unp.h` 및 관련 라이브러리/지원 코드가 준비되어 있다는 전제에서 C compiler로 빌드할 수 있습니다.

예시:

```bash
gcc daytime_client.c -o daytime_client
gcc daytime_server.c -o daytime_server
gcc echo_client.c -o echo_client
gcc echo_server.c -o echo_server
```

---

## Run Example

### Echo Server

```bash
./echo_server <port>
```

### Echo Client

```bash
./echo_client <server_ip> <port>
```

Client에서 문자열을 입력하면 Server가 동일한 문자열을 반환합니다.

```text
> hello
Message from server: hello
```

---

## What I Learned

- TCP Client와 Server의 역할 차이
- `socket → bind → listen → accept`로 이어지는 TCP Server 연결 과정
- `socket → connect`로 이어지는 TCP Client 연결 과정
- `read()` / `write()` 기반 양방향 데이터 송수신
- IPv4 주소와 Port를 socket 구조체에 설정하는 방법
- Host byte order와 Network byte order의 차이
- 연결 socket과 listening socket의 역할 차이
- TCP가 byte stream 기반으로 데이터를 전달한다는 기본 특성