/*
    TCP Daytime Client

- Original Source:
    W. Richard Stevens, Bill Fenner, and Andrew M. Rudoff, UNIX Network Programming Vol. 1: The Sockets Networking API, 3rd Ed., Addison-Wesley, 2004.

- Revised by:
    Prof. Heejun Roh, Ph.D. (Inha University)

 */
#include "unp.h"

/*

* 프로그램에 관한 배경 지식
  * Daytime Protocol은 RFC 867에 정의됨 <https://en.wikipedia.org/wiki/Daytime_Protocol>
  * 오래 전 프로토콜이라 요즘은 Network Time Protocol (NTP)를 쓰는 게 보통이지만, 이 프로토콜이 훨씬 간단해서 공부용으로 많이 쓰임
  * 미국 국립표준기술연구소 (NIST)에서는 아직도 Daytime Protocol 서버를 운용하고 있으며, <https://tf.nist.gov/tf-cgi/servers.cgi>에 그 리스트가 있음
    * 위의 사이트에서 IP 주소를 가져와 인자로 넣을 것
  * Daytime Protocol은 시간을 ASCII 포맷으로 전달하는 것 외에는 아무런 제한 조건이 없어서, 서버마다 다른 시간 포맷을 사용하고 있음
    * 시간 위의 사이트에서 쓰는 시간 포맷은 <https://www.nist.gov/pml/time-and-frequency-division/services/internet-time-service-its>에 설명이 되어 있음 (RFC-867로 검색)
    * NIST의 출력 결과는 문서에 적혀있는 것 외에도 맨 시작에 line feed ('\n')을 추가로 출력함. 아래는 서버에서 보내온 DAYTIME Response를 Wireshark에서 캡처한 것으로, 58671 앞에 0a ('\n')가 포함되어 있음을 볼 수 있음

```
0000   00 80 20 13 ef fb 04 a1 51 bc 65 7e 08 00 45 00   .. .....Q.e~..E.
0010   00 5b 00 00 40 00 2f 06 42 4d 84 a3 60 01 c0 a8   .[..@./.BM..`...
0020   64 03 00 0d d4 a1 11 1a f8 ef e6 be 5b b2 50 18   d...........[.P.
0030   04 02 66 4d 00 00 0a 35 38 36 37 31 20 31 39 2d   ..fM...58671 19-
0040   30 37 2d 30 37 20 30 36 3a 33 30 3a 31 37 20 35   07-07 06:30:17 5
0050   30 20 30 20 30 20 33 34 36 2e 36 20 55 54 43 28   0 0 0 346.6 UTC(
0060   4e 49 53 54 29 20 2a 20 0a                        NIST) * .
```

* 사용된 라이브러리 관련 정보 (UNIX 기준)
  * NOTE: (<>로 감싸진) 라이브러리의 헤더 파일에 포함된 정의나 선언은 라이브러리 구현에 따라 실제 정의/선언 위치가 아래와 다를 수 있음. 하지만 표준 상으로 보통 아래와 같이 정의한 것을 찾으면 문제가 없음
  *
  * 데이터 타입
    * struct sockaddr         : <sys/socket.h>에 정의됨 (공간을 줄이고자 "unp.h"에 SA로 매크로로 정의됨)
    * struct sockaddr_in      : <netinet/in.h>에 정의됨
    *
  * 매크로 (값)
    * MAXLINE                 : "unp.h"에 매크로로 정의됨
    * AF_INET                 : <sys/socket.h>에 정의됨
    * SOCK_STREAM             : <sys/socket.h>에 정의됨
    *
  * 함수 및 매크로 함수
    * socket(2)               : <sys/socket.h>에 선언됨 (역사적인 이유로 <sys/types.h>도 인클루드 하는 게 바람직함)
    * htons(3)                : <arpa/inet.h>에 선언됨
    * inet_pton(3)            : <arpa/inet.h>에 선언됨
    * connect(2)              : <sys/socket.h>에 선언됨 (역사적인 이유로 <sys/types.h>도 인클루드 하는 게 바람직함)
    * read(2)                 : <unistd.h>에 선언됨
    * fputs(3)                : <stdio.h>에 선언됨
    * exit(3)                 : <stdlib.h>에 선언됨
    *
    * bzero()                 : "unp.h"에 매크로로 정의됨 (해당 정의를 살펴볼 것)
    *
    * err_quit()              : "unp.h"에 선언됨, "unp_error.c"에 정의됨
    * err_sys()               : "unp.h"에 선언됨, "unp_error.c"에 정의됨
 */

int main(int argc, char *argv[])
{
    int sockfd, n;               // sockfd 소켓 디스크립터 (번호표), n은 시간 정보의 크기 (바이트 단위)
    char recvline[MAXLINE + 1];  // 서버로부터 받을 시간 정보를 저장할 버퍼
    struct sockaddr_in servaddr; // 서버의 주소를 저장한 인터넷 소켓 주소 구조체

    // 시간 서버의 IP 주소가 인자로 주어졌는지를 체크
    if (argc != 2)
        err_quit("usage: %s <IPaddress>", argv[0]);

        // Windows Sockets API Version 2.2를 사용하기 위한 부분
        // <https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup>
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        err_quit("WSAStartup failed with error: %d\n", err);
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup();
        err_quit("Could not find a usable version of Winsock.dll\n");
    }
#endif

    // TCP 소켓을 생성해 socket descriptor를 받아와서 sockfd에 저장
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // 가끔 구식 책이나 BSD 계열에서 AF_INET을 PF_INET이라고 적지만, 최근 표준에서는 AF_INET을 쓰는 걸로 통일
        err_sys("socket error");                        // 어느 쪽으로 써도 문제는 없음

    // 서버 주소에 대한 구조체 servaddr에 주소를 설정함
    bzero(&servaddr, sizeof(servaddr));                       // 0으로 채움
    servaddr.sin_family = AF_INET;                            // Internet Protocol Suite (IPv4)를 사용
    servaddr.sin_port = htons(13);                            /* daytime server port number = 13임 */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) // (argv[1] 문자열)에 저장된 dotted decimal 형식의 서버 주소를 32비트 이진수의 형태로 변환함
        err_quit("inet_pton error for %s", argv[1]);

    // servaddr에 저장된 주소로 연결을 시도함
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) // 공간을 줄이기 위해 socket sockaddr는 SA로 치환함. connect는 SA * 타입을 받고 싶은데
        err_sys("connect error");                               // &servaddr는 struct sockaddr_in* 타입이라 타입이 맞지 않아서 타입 캐스팅 필요

    // 이 위치에 도작했다는 것은 TCP 연결 설립이 성공했다는 이야기

    // 연결된 소켓을 통해 서버가 보낸 바이트 열 (즉, 읽을 문장)이 있으면
#ifdef _WIN32
    while ((n = recv(sockfd, recvline, MAXLINE, 0)) > 0)
    { // 파일을 읽듯이 최대 MAXLINE 크기만큼 읽어서 recvline에 저장함
#else
    while ((n = read(sockfd, recvline, MAXLINE)) > 0)
    { // 파일을 읽듯이 최대 MAXLINE 크기만큼 읽어서 recvline에 저장함
#endif
        recvline[n] = 0; /* null terminate */ // buffer overflow를 막기 위해 마지막 글자는 널 캐릭터로 바꿈
        if (fputs(recvline, stdout) == EOF)   // stdout에 recvline을 밀어넣음 (화면에 그대로 출력하는 것임)
            err_sys("fputs error");           // read는 EOF가 등장하면 0이 되고 while 루프를 나가게 되므로, fputs는 EOF가 나올 수 없음. read가 EOF를 읽었다면 문제가 있다는 것임
    }

    if (n < 0) // 정상적으로 루프를 종료했다면 n == 0이어야 하는데, n < 0이라는 건 read 함수가 에러가 났다는 의미임
        err_sys("read error");

#ifdef _WIN32
    /* The Winsock DLL is acceptable. Proceed to use it. */
    /* Add network programming using Winsock here */
    /* then call WSACleanup when done using the Winsock dll */
    WSACleanup();
#endif

    exit(0);
}