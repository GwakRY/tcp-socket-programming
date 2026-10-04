# TCP Socket Programming

컴퓨터 네트워크 전공 과목의 C/Linux 소켓 프로그래밍 실습입니다.
Daytime과 Echo Client/Server를 통해 TCP 연결, IPv4 주소 처리와 스트림 송수신을 학습했습니다.

## 구현과 출처

| 프로그램 | 기능 | 코드 |
|---|---|---|
| Daytime Server | 현재 시각 문자열 전송 후 연결 종료 | [daytime_server.c](daytime/daytime_server.c) |
| Daytime Client | 서버의 시각 데이터를 EOF까지 수신 | [daytime_client.c](daytime/daytime_client.c) |
| Echo Server | 수신한 바이트를 반환, 연결 종료 후 다음 접속 처리 | [echo_server.c](echo/echo_server.c) |
| Echo Client | 입력 문자열 전송 후 전송 길이만큼 응답 수신 | [echo_client.c](echo/echo_client.c) |

Daytime 소스는 파일 주석에 명시된 **UNIX Network Programming(3판)의 예제와 수업 수정 코드**를 기반으로 합니다. 기본 예제 전체를 독자 창작으로 주장하지 않습니다. Echo는 수업 실습 코드이며, 아래 실행·송수신 보완은 포트폴리오 정리 과정의 후속 수정입니다.

기존 저장소에는 `unp.h`와 오류 처리 지원 코드가 없어 독립 빌드가 불가능했습니다. [common/socket_support.h](common/socket_support.h)와 [socket_support.c](common/socket_support.c)는 필요한 POSIX 헤더·상수·오류 처리·포트 검증·전송 함수를 **새로 작성한 Linux용 지원 코드**입니다. 누락된 원본 UNP 라이브러리를 복원한 파일이 아닙니다.

## Build

Linux에서 GCC와 GNU Make가 필요합니다. Windows에서는 WSL의 Linux 터미널을 사용하세요. 현재 Makefile은 Linux용이며 Windows 네이티브 빌드는 검증하지 않았습니다.

Ubuntu/Debian 도구 설치:

```bash
sudo apt update
sudo apt install build-essential
```

저장소 다운로드(Git 필요):

```bash
git clone https://github.com/GwakRY/tcp-socket-programming.git
cd tcp-socket-programming
make
```

Git 없이 **Code → Download ZIP**으로 내려받아 압축을 풀어도 됩니다. `make`와 아래 명령은 저장소 최상위 디렉터리에서 실행합니다. 외부 UNP 라이브러리 설치는 필요하지 않습니다.

## Run

서버와 클라이언트를 **서로 다른 터미널**에서 실행합니다. 서버는 로컬 주소 `127.0.0.1`에서만 접속을 받습니다.

### Daytime

터미널 1:

```bash
./build/daytime_server
```

터미널 2:

```bash
./build/daytime_client 127.0.0.1
```

시각 문자열이 출력된 뒤 클라이언트가 종료됩니다. 출력 예시이며 실제 시각은 실행 환경에 따라 다릅니다.

```text
Sun Oct  4 15:00:00 2026
```

기본 포트는 **1313**입니다. 원래 예제의 13번 대신 일반 사용자 권한으로 실행 가능한 포트를 사용합니다. 다른 포트를 사용할 때는 양쪽에 같은 값을 지정하세요.

```bash
./build/daytime_server 1513
./build/daytime_client 127.0.0.1 1513
```

### Echo

터미널 1:

```bash
./build/echo_server 9000
```

터미널 2:

```bash
./build/echo_client 127.0.0.1 9000
```

클라이언트에서 `hello`를 입력하면 다음과 같이 반환됩니다.

```text
 > hello
Message from server: hello
```

- 클라이언트: `q` / `Q` 입력 또는 `Ctrl+D`로 종료
- 서버: `Ctrl+C`로 종료
- `bind error: Address already in use`: 해당 포트의 기존 서버를 종료하거나 다른 포트 사용
- `connect error: Connection refused`: 서버 실행 여부와 양쪽 포트 확인

## 송수신 처리와 후속 개선

**2026-10-04 후속 수정**이며 프로젝트 당시 구현과 구분합니다.

- Echo Server: 초기화되지 않은 버퍼의 `strlen()` 대신 실제 버퍼 크기로 읽고, `read()` 결과 길이만큼 반환합니다.
- Echo Client: 입력 EOF와 서버 조기 종료를 처리하고, 남은 응답 길이 이내로 읽어 버퍼 범위를 제한합니다.
- 공통: 부분 전송 시 남은 바이트를 반복 전송하고 `EINTR`을 재시도합니다. 전송 시 `MSG_NOSIGNAL`을 사용합니다.
- 실행: 포트 범위 검증, Makefile, 빌드 결과 제외, Daytime 포트 선택 및 로컬 주소 바인딩을 추가했습니다.

TCP는 메시지 경계를 보존하지 않는 바이트 스트림입니다. Echo Client는 요청한 길이만큼 응답을 누적하며, 서버는 문자열 종료 문자 대신 수신 바이트 수로 응답합니다.

학습용 순차 처리 서버이므로 연결 하나를 처리하는 동안 다음 연결 처리가 지연됩니다. 동시 접속 처리, 타임아웃, 인증·암호화는 포함하지 않습니다.

## 실행 검증

Ubuntu 24.04.3 LTS / GCC 13.3.0 / GNU Make 4.3 / Python 3 환경에서 프로그램 4개의 빌드와 [로컬 통합 테스트](tests/smoke.py) 6개를 확인했습니다.

```bash
make test
```

Python 표준 라이브러리만 사용하며, 외부 서버 없이 `127.0.0.1`의 임시 포트에서 검증합니다.

| 검증 | 확인 내용 |
|---|---|
| Daytime | 실제 C 클라이언트의 시각 수신, 반복 접속 |
| Echo 문자열 | 여러 줄 송수신, 종료 명령 |
| 입력 종료 | 즉시 종료, EOF, 개행 없는 마지막 입력 |
| 스트림 | 10,240바이트 바이너리 데이터 분할 송수신, 재접속 |
| 잘못된 인자 | 포트 범위·문자열 및 잘못된 IPv4 주소 |
| 불완전한 응답 | 서버가 응답 도중 종료하면 클라이언트 오류 종료 |

실제 외부 네트워크·고부하·동시 접속 성능을 검증한 결과는 아닙니다.

빌드 결과 정리:

```bash
make clean
```
