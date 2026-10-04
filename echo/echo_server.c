/* Linux echo exercise; buffer/EOF/send handling revised 2026-10-04. */
#include "socket_support.h"
int main(int argc, char *argv[]) {
    if (argc != 2) err_quit("Usage: %s <port>", argv[0]);
    unsigned short port = parse_port(argv[1]);
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) err_sys("socket error");
    prepare_listener(listenfd);
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (bind(listenfd, (SA *)&address, sizeof(address)) < 0) err_sys("bind error");
    if (listen(listenfd, LISTENQ) < 0) err_sys("listen error");
    for (;;) {
        int fd = accept(listenfd, NULL, NULL);
        if (fd < 0) {
            if (errno == EINTR || errno == ECONNABORTED) continue;
            err_sys("accept error");
        }
        char buffer[MAXLINE];
        for (;;) {
            ssize_t length = read(fd, buffer, sizeof(buffer));
            if (length < 0 && errno == EINTR) continue;
            if (length <= 0) break;
            if (write_all(fd, buffer, (size_t)length) < 0) break;
        }
        close(fd);
    }
}
