#include "unp.h"

int main(int argc, char *argv[])
{
    int sockfd, n;
    int recv_len, recv_cnt;               // sockfd 소켓 디스크립터 (번호표), n은 시간 정보의 크기 (바이트 단위)
    char recvline[100];  // 서버로부터 받을 시간 정보를 저장할 버퍼
    struct sockaddr_in servaddr; // 서버의 주소를 저장한 인터넷 소켓 주소 구조체

    // 시간 서버의 IP 주소가 인자로 주어졌는지를 체크
    if (argc != 3)
        err_quit("Usage: %s <IP> <port>", argv[0]);

    // TCP 소켓을 생성해 socket descriptor를 받아와서 sockfd에 저장
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // 가끔 구식 책이나 BSD 계열에서 AF_INET을 PF_INET이라고 적지만, 최근 표준에서는 AF_INET을 쓰는 걸로 통일
        err_sys("socket error");                        // 어느 쪽으로 써도 문제는 없음

    // 서버 주소에 대한 구조체 servaddr에 주소를 설정함
    bzero(&servaddr, sizeof(servaddr));                       // 0으로 채움
    servaddr.sin_family = AF_INET;                            // Internet Protocol Suite (IPv4)를 사용
    servaddr.sin_port = htons(atoi(argv[2]));                            /* daytime server port number = 13임 */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) // (argv[1] 문자열)에 저장된 dotted decimal 형식의 서버 주소를 32비트 이진수의 형태로 변환함
        err_quit("inet_pton error for %s", argv[1]);

    // servaddr에 저장된 주소로 연결을 시도함
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) // 공간을 줄이기 위해 socket sockaddr는 SA로 치환함. connect는 SA * 타입을 받고 싶은데
        err_sys("connect error");                               // &servaddr는 struct sockaddr_in* 타입이라 타입이 맞지 않아서 타입 캐스팅 필요

    // 이 위치에 도작했다는 것은 TCP 연결 설립이 성공했다는 이야기
    
    while(1){
        fputs(" > ", stdout);
        fgets(recvline, 100, stdin);
        if(!strcmp(recvline,"q\n") || !strcmp(recvline,"Q\n"))break;
        n = write(sockfd, recvline, strlen(recvline));

        recv_len = 0;
        while(recv_len < n){
            recv_cnt =read(sockfd, &recvline[recv_len], 10);
            if(recv_cnt == -1){
                err_sys("read error");
            }
            recv_len +=recv_cnt;
        }
        
        recvline[recv_len] = 0;
        printf("Message from server: %s\n", recvline);
    }
    

    // 연결된 소켓을 통해 서버가 보낸 바이트 열 (즉, 읽을 문장)이 있으면
    /*
    printf("Message from server: ");
    n = read(sockfd, recvline, 49);
    recvline[n] = 0; // null terminate  // buffer overflow를 막기 위해 마지막 글자는 널 캐릭터로 바꿈
    if (fputs(recvline, stdout) == EOF)   // stdout에 recvline을 밀어넣음 (화면에 그대로 출력하는 것임)
        err_sys("fputs error");           // read는 EOF가 등장하면 0이 되고 while 루프를 나가게 되므로, fputs는 EOF가 나올 수 없음. read가 EOF를 읽었다면 문제가 있다는 것임
    printf(" \n");
    */

    if (recv_cnt < 0) // 정상적으로 루프를 종료했다면 n == 0이어야 하는데, n < 0이라는 건 read 함수가 에러가 났다는 의미임
        err_sys("read error");

    if (close(sockfd) == -1) // accept로 얻은 소켓 (sockfd)을 닫음
        err_sys("close error");

    exit(0);
}