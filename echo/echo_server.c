#include "unp.h"
#include <time.h>

int main(int argc, char *argv[]) {
    int listenfd, connfd; // client를 기다리는 listenfd와, 연결 수립 후 사용하는 connfd
    char buff[1];   // client에 보낼 내용을 저장하는 buffer
    struct sockaddr_in servaddr; // server가 client와의 연결 설립을 허용할 'server의' 주소 범위를 저장함
    time_t ticks;

    int str_len;

    if (argc != 2)
        err_quit("Usage: %s <port>", argv[0]);

    // TCP 소켓을 생성해 socket descriptor를 받아와서 sockfd에 저장
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_sys("socket error");

    // 서버 주소에 대한 구조체 servaddr에 주소를 설정함
    bzero(&servaddr, sizeof(servaddr));           // 0으로 채움
    servaddr.sin_family = AF_INET;                // Internet Protocol Suite를 사용
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0으로 채우는 것과 동일함
    servaddr.sin_port = htons(atoi(argv[1]));                /* daytime server port number */

    // servaddr에 저장된 주소를 소켓에 bind함
    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0) // 공간을 줄이기 위해 socket sockaddr는 SA로 치환함. connect는 SA * 타입을 받아야 하는데 &servaddr는 struct sockaddr_in* 타입으로 다른 상황이므로, 타입 캐스팅이 필요함
        err_sys("bind error");

    // bind한 소켓을 통해 client의 접속을 대기 (listen)함
    int backlog = 5;  // listen 함수가 리턴될 때까지 받을 수 있는 클라이언트의 최대 수

    if (listen(listenfd, backlog) < 0) err_sys("listen error");




    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

        while((str_len=read(connfd,buff,strlen(buff)))!=0){
            write(connfd, buff, str_len);
        }

        /*
        snprintf(buff, 46, "Hello, World! This is CSE3212-002, Inha Univ.");

        // 문장을 씀
        if (write(connfd, buff, strlen(buff)) != strlen(buff)) { // 파일을 쓰듯이 buff의 문자열을 읽어 buff 길이만큼 송신함
            err_sys("send error");
        }
        */

        if (close(connfd) == -1) // accept로 얻은 소켓 (connfd)을 닫음
            err_sys("close error");
    }

    exit(0);
}