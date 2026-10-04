#include "socket_support.h"

static void report(const char *format, va_list args, int error) {
    vfprintf(stderr, format, args);
    if (error) fprintf(stderr, ": %s", strerror(error));
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

void err_quit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    report(format, args, 0);
    va_end(args);
}

void err_sys(const char *format, ...) {
    int error = errno;
    va_list args;
    va_start(args, format);
    report(format, args, error);
    va_end(args);
}

unsigned short parse_port(const char *text) {
    char *end;
    errno = 0;
    long port = strtol(text, &end, 10);
    if (errno || text == end || *end || port < 1 || port > 65535)
        err_quit("port must be an integer from 1 to 65535");
    return (unsigned short)port;
}

int write_all(int fd, const void *buffer, size_t length) {
    const char *bytes = buffer;
    while (length > 0) {
        ssize_t n = send(fd, bytes, length, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        bytes += n;
        length -= (size_t)n;
    }
    return 0;
}

void prepare_listener(int fd) {
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        err_sys("setsockopt error");
}
