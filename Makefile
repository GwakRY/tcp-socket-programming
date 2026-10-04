CC = gcc
CFLAGS = -O2 -Wall -Wextra
CPPFLAGS = -Icommon
PROGRAMS = build/daytime_client build/daytime_server build/echo_client build/echo_server

.PHONY: all clean test
all: $(PROGRAMS)

build:
	mkdir -p build

build/daytime_%: daytime/daytime_%.c common/socket_support.c common/socket_support.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< common/socket_support.c -o $@

build/echo_%: echo/echo_%.c common/socket_support.c common/socket_support.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< common/socket_support.c -o $@

test: all
	python3 tests/smoke.py

clean:
	rm -rf build
