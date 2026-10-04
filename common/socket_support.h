/* Independently written Linux/POSIX support for the 2026-10-04 follow-up.
 * This is not a restored copy of UNP's unp.h or unp_error.c. */
#ifndef SOCKET_SUPPORT_H
#define SOCKET_SUPPORT_H
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAXLINE 4096
#define LISTENQ 5
#define SA struct sockaddr
void err_quit(const char *format, ...);
void err_sys(const char *format, ...);
unsigned short parse_port(const char *text);
int write_all(int fd, const void *buffer, size_t length);
void prepare_listener(int fd);
#endif
