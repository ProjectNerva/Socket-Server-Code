# Socket-Server-Code

A collection of C++ TCP socket server programs exploring progressively more complex networking and concurrency patterns. Each server is designed to work with its counterpart in the [Socket-Client-Code](https://github.com/ProjectNerva/Socket-Client-Code) repository.

## Programs

| File | Description |
|------|-------------|
| `chat_socket_server.cpp` | Interactive chat server — takes a port argument, accepts one client, bidirectional messaging with session stats |
| `multiSC_socket_server.cpp` | Multi-client server using `std::thread` — listens on port 8080, spawns a thread per client and echoes messages back |
| `multithreadP_socket_server.cpp` | Full chatroom server using pthreads — broadcasts messages to all connected clients, color-coded output, named users, chat history saved on exit |

## Client-Server Pairing

| Server | Client |
|--------|--------|
| `chat_socket_server.cpp` | `chat_socket_client.cpp` |
| `multiSC_socket_server.cpp` | `multiSC_socket_client.cpp` |
| `multithreadP_socket_server.cpp` | `multithreadP_socket_client.cpp` |

## Build

Each file compiles independently. Use `-lpthread` for the threaded programs:

```bash
# Single-client server
g++ -o chat_socket_server chat_socket_server.cpp

# Multi-client servers
g++ -o multiSC_socket_server multiSC_socket_server.cpp -lpthread
g++ -o multithreadP_socket_server multithreadP_socket_server.cpp -lpthread
```

## Usage

**chat_socket_server** — pass a port number:
```bash
./chat_socket_server 8080
```
Accepts one client. Type `exit` on either end to end the session. Prints bytes read/written and elapsed time on close.

**multiSC_socket_server** — no arguments, hardcoded to port 8080:
```bash
./multiSC_socket_server
```
Accepts multiple clients concurrently. Echoes each message back to the sender.

**multithreadP_socket_server** — pass a port number:
```bash
./multithreadP_socket_server 8080
```
Full multi-user chatroom. Broadcasts all messages to every connected client. Press Ctrl+C to shut down and save chat history to `test_ChatHistory/`.

## Requirements

- Clients from the [Socket-Client-Code](https://github.com/ProjectNerva/Socket-Client-Code) repo
- Linux or macOS
- C++11 or later (`g++` or `clang++`)
- POSIX thread support (`-lpthread`)