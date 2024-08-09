from __future__ import annotations

import concurrent.futures
import signal
import socket
import subprocess
from typing import Optional, Tuple

CLIENT_TIMEOUT: float = 120.0


def handle_connection(client_socket: socket.socket,
                      client_addr: Tuple[str, int]):
    print(f'Handling connection for {client_addr}...')
    client_socket.set_inheritable(True)
    with client_socket:
        try:
            signal.signal(signal.SIGINT, signal.default_int_handler)
            print(f'Starting subprocess...')
            heap_miner_subprocess: subprocess.Popen = subprocess.Popen(
                args=['heap_miner', str(client_socket.fileno())],
                close_fds=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                pass_fds=[client_socket.fileno()],
                text=True)

            print(f'Waiting for subprocess...', flush=True)
            stdout: Optional[str]
            stderr: Optional[str]
            stdout, stderr = heap_miner_subprocess.communicate(timeout=CLIENT_TIMEOUT)
            returncode: Optional[int] = heap_miner_subprocess.poll()
            if returncode is None:
                heap_miner_subprocess.kill()
                print(f'Killed heap_miner subprocess after timeout')

            else:
                print(f'Heap miner subprocess exited with code {returncode}')
                print(f'stdout: {stdout}')
                print(f'stderr: {stderr}')

        except OSError as socket_error:
            print(f'Socket error occurred: {socket_error}')

        except KeyboardInterrupt:
            heap_miner_subprocess.kill()
            print(f'Killed heap_miner subprocess after SIGINT')

        finally:
            signal.signal(signal.SIGINT, signal.SIG_IGN)

    print(f'Handled connection!')


def ignore_sigint():
    signal.signal(signal.SIGINT, signal.SIG_IGN)


def main() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        try:
            server_socket.bind(('', 1234))
            server_socket.listen()

        except OSError as socket_error:
            print(f'Socket error occurred: {socket_error}')
            return 1

        with concurrent.futures.ProcessPoolExecutor(max_workers=2, initializer=ignore_sigint) as pool:
            print(f'Starting heap miner server...')
            try:
                while True:
                    client_socket: socket.socket
                    client_addr: Tuple[str, int]
                    client_socket, client_addr = server_socket.accept()
                    with client_socket:
                        pool.submit(handle_connection, client_socket, client_addr)

            except KeyboardInterrupt:
                print(f'Received keyboard interrupt, exiting...')
                pool.shutdown(wait=True,
                              cancel_futures=True)
                return 0


if __name__ == '__main__':
    main()
