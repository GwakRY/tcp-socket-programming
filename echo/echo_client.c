/* Linux echo exercise; bounded reads and disconnect handling revised 2026-10-04. */
#include "socket_support.h"
int main(int argc, char *argv[]) {
    if (argc != 3) err_quit("Usage: %s <IPv4> <port>", argv[0]);
    unsigned short port = parse_port(argv[2]);
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (inet_pton(AF_INET, argv[1], &address.sin_addr) != 1)
        err_quit("invalid IPv4 address: %s", argv[1]);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) err_sys("socket error");
    if (connect(fd, (SA *)&address, sizeof(address)) < 0) err_sys("connect error");
    char line[MAXLINE];
    for (;;) {
        fputs(" > ", stdout);
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        if (!strcmp(line, "q\n") || !strcmp(line, "Q\n")) break;
        size_t expected = strlen(line);
        if (write_all(fd, line, expected) < 0) err_sys("send error");
        size_t received = 0;
        while (received < expected) {
            ssize_t n = read(fd, line + received, expected - received);
            if (n < 0 && errno == EINTR) continue;
            if (n < 0) err_sys("read error");
            if (n == 0) err_quit("server disconnected before complete echo");
            received += (size_t)n;
        }
        line[received] = '\0';
        printf("Message from server: %s", line);
    }
    if (close(fd) < 0) err_sys("close error");
    return 0;
}
