/* 
    TCP Daytime Server

- Original Source:
    W. Richard Stevens, Bill Fenner, and Andrew M. Rudoff, UNIX Network Programming Vol. 1: The Sockets Networking API, 3rd Ed., Addison-Wesley, 2004.

- Revised by:
    Prof. Heejun Roh, Ph.D. (Inha University)

 */
#ifdef _WIN32 // Windows Headers
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "unp.h"
#include <time.h>

/* 

* 프로그램에 관한 배경 지식
  * Daytime Protocol은 RFC 867에 정의됨 <https://en.wikipedia.org/wiki/Daytime_Protocol>
  * 오래 전 프로토콜이라 요즘은 Network Time Protocol (NTP)를 쓰는 게 보통이지만, 이 프로토콜이 훨씬 간단해서 공부용으로 많이 쓰임
  * Daytime Protocol은 시간을 ASCII 포맷으로 전달하는 것 외에는 아무런 제한 조건이 없어서, 서버마다 다른 시간 포맷을 사용하고 있음
    * 여기서는 <time.h>에 정의된 ctime(3)이라는 함수를 이용할 것임


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
    * INADDR_ANY              : <netinet/in.h>에 정의됨
    * 
  * 함수 및 매크로 함수
    * socket(2)               : <sys/socket.h>에 선언됨 (역사적인 이유로 <sys/types.h>도 인클루드 하는 게 바람직함)
    * htonl(3), htons(3)      : <arpa/inet.h>에 선언됨
    * bind(2)                 : <sys/socket.h>에 선언됨 (역사적인 이유로 <sys/types.h>도 인클루드 하는 게 바람직함)
    * accept(2)               : <sys/socket.h>에 선언됨 (역사적인 이유로 <sys/types.h>도 인클루드 하는 게 바람직함)
    * snprintf(3)             : <stdio.h>에 선언됨
    * time(2)                 : <time.h>에 선언됨
    * write(2)                : <unistd.h>에 선언됨
    * close(2)                : <unistd.h>에 선언됨
    * exit(3)                 : <stdlib.h>에 선언됨
    *
    * bzero()                 : "unp.h"에 매크로로 정의됨 (해당 정의를 살펴볼 것)
    * 
    * err_quit()              : "unp.h"에 선언됨, "unp_error.c"에 정의됨
    * err_sys()               : "unp.h"에 선언됨, "unp_error.c"에 정의됨
 */

int main(int argc, char *argv[]) {
    int listenfd, connfd; // client를 기다리는 listenfd와, 연결 수립 후 사용하는 connfd
    char buff[MAXLINE];   // client에 보낼 내용을 저장하는 buffer
    struct sockaddr_in servaddr; // server가 기다리려는 client의 주소 범위를 저장함
    time_t ticks;

    // Windows Sockets API Version 2.2를 사용하기 위한 부분
    // <https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup>
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        err_quit("WSAStartup failed with error: %d\n", err);
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup();
        err_quit("Could not find a usable version of Winsock.dll\n");
    }
#endif

    // TCP 소켓을 생성해 socket descriptor를 받아와서 sockfd에 저장
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_sys("socket error");

    // 서버 주소에 대한 구조체 servaddr에 주소를 설정함
    bzero(&servaddr, sizeof(servaddr));           // 0으로 채움
    servaddr.sin_family = AF_INET;                // Internet Protocol Suite를 사용
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0으로 채우는 것과 동일함
    servaddr.sin_port = htons(13);                /* daytime server port number */

    // servaddr에 저장된 주소를 소켓에 bind함
    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0) // 공간을 줄이기 위해 socket sockaddr는 SA로 치환함. connect는 SA * 타입을 받아야 하는데 &servaddr는 struct sockaddr_in* 타입으로 다른 상황이므로, 타입 캐스팅이 필요함
        err_sys("bind error");

    // bind한 소켓을 통해 client의 접속을 대기 (listen)함
    char *ptr;
    int backlog = LISTENQ;  // listen 함수가 리턴될 때까지 받을 수 있는 클라이언트의 최대 수

    if ((ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);

    if (listen(listenfd, backlog) < 0)
        err_sys("listen error");

    for (;;) {  // 여기서 for를 사용한 것은 서버가 여러 클라이언트를 계속 받을 수 있도록 무한 루프를 도는 것임.
    again:
        if ((connfd = accept(listenfd, (SA *)NULL, NULL)) < 0) { // (SA *)NULL 부분엔 원래 client의 주소 정보를 저장할 구조체의 주소가 들어가는데, 이 프로그램은 그 구조체를 사용하지 않아서 NULL을 대입함
#ifdef EPROTO                                             /* Protocol error */
            if (errno == EPROTO || errno == ECONNABORTED) /* Software caused connection abort */
#else
            if (errno == ECONNABORTED) /* Software caused connection abort */
#endif
                goto again;
            else
                err_sys("accept error");
        }

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));

        // 문장을 씀
#ifdef _WIN32
        if (send(connfd, buff, strlen(buff), 0) != strlen(buff)) { // 파일을 쓰듯이 buff의 문자열을 읽어 buff 길이만큼 송신함
#else
        if (write(connfd, buff, strlen(buff)) != strlen(buff)) { // 파일을 쓰듯이 buff의 문자열을 읽어 buff 길이만큼 송신함
#endif
            err_sys("send error");
        }

#ifdef _WIN32
        if (closesocket(connfd) == SOCKET_ERROR)
#else
        if (close(connfd) == -1) // accept로 얻은 소켓 (connfd)을 닫음
#endif
            err_sys("close error");
    }

#ifdef _WIN32
    /* The Winsock DLL is acceptable. Proceed to use it. */
    /* Add network programming using Winsock here */
    /* then call WSACleanup when done using the Winsock dll */
    WSACleanup();
#endif

    exit(0);
}