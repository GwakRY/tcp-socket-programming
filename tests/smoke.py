"""Loopback integration tests; Python stdlib, no external server required."""
from contextlib import contextmanager
from pathlib import Path
import socket
import subprocess
import threading
import time
import unittest

BUILD = Path(__file__).resolve().parents[1] / "build"


@contextmanager
def server(name):
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
    process = subprocess.Popen([str(BUILD / name), str(port)],
                               stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    try:
        deadline = time.monotonic() + 3
        while True:
            if process.poll() is not None:
                raise AssertionError(process.stderr.read().decode())
            try:
                connection = socket.create_connection(("127.0.0.1", port), timeout=.1)
                connection.close()
                break
            except OSError:
                if time.monotonic() > deadline:
                    raise AssertionError("server did not start")
                time.sleep(.02)
        yield port
    finally:
        process.terminate()
        try:
            process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        process.stderr.close()


class SocketTests(unittest.TestCase):
    def client(self, name, port, data=""):
        return subprocess.run([str(BUILD / name), "127.0.0.1", str(port)],
                              input=data, text=True, capture_output=True, timeout=4)

    def test_daytime_client_and_repeated_connections(self):
        with server("daytime_server") as port:
            for _ in range(2):
                result = self.client("daytime_client", port)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertRegex(result.stdout, r"\w{3} \w{3} +\d+ \d{2}:\d{2}:\d{2} \d{4}")

    def test_echo_multiple_lines_and_quit(self):
        with server("echo_server") as port:
            result = self.client("echo_client", port, "hello\nworld\nq\n")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("Message from server: hello", result.stdout)
            self.assertIn("Message from server: world", result.stdout)

    def test_echo_client_eof_and_immediate_quit(self):
        with server("echo_server") as port:
            for data in ("", "Q\n", "last line without newline"):
                result = self.client("echo_client", port, data)
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_echo_binary_fragmentation_and_reconnect(self):
        payload = bytes(range(256)) * 40
        with server("echo_server") as port:
            for _ in range(2):
                with socket.create_connection(("127.0.0.1", port), timeout=2) as connection:
                    for start in range(0, len(payload), 73):
                        connection.sendall(payload[start:start + 73])
                    connection.shutdown(socket.SHUT_WR)
                    received = bytearray()
                    while chunk := connection.recv(97):
                        received.extend(chunk)
                    self.assertEqual(received, payload)

    def test_invalid_ports_and_ipv4(self):
        for name in ("daytime_server", "echo_server"):
            for port in ("0", "65536", "abc", "-1"):
                result = subprocess.run([str(BUILD / name), port], capture_output=True, timeout=2)
                self.assertNotEqual(result.returncode, 0)
        for name in ("daytime_client", "echo_client"):
            result = subprocess.run([str(BUILD / name), "invalid", "1313"],
                                    capture_output=True, timeout=2)
            self.assertNotEqual(result.returncode, 0)

    def test_echo_client_detects_incomplete_reply(self):
        with socket.socket() as listener:
            listener.bind(("127.0.0.1", 0))
            listener.listen(1)
            listener.settimeout(3)
            port = listener.getsockname()[1]

            def incomplete_reply():
                with listener.accept()[0] as connection:
                    connection.recv(100)
                    connection.sendall(b"x")

            worker = threading.Thread(target=incomplete_reply, daemon=True)
            worker.start()
            result = self.client("echo_client", port, "hello\n")
            worker.join(timeout=3)
            self.assertFalse(worker.is_alive())
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("server disconnected", result.stderr)


if __name__ == "__main__":
    unittest.main(verbosity=2)
