# IRC 📡  
**RFC-Compliant Internet Relay Chat Server in C++**

IRC is a network-based project built as part of the 42 School curriculum. It implements a multi-client **IRC server** in C++, following the core specifications of the **RFC 1459** standard. The server handles connections using **non-blocking sockets** and **epoll**, allowing multiple clients to communicate in real time through IRC channels.
Through this project, I strengthened my understanding of **network architecture**, **protocol handling**, and how real-time communication systems operate at the socket level.

## ✨ Key Features

- Built in modern **C++** with custom TCP socket server  
- Fully compliant with **RFC 1459** (core IRC protocol)  
- Supports:
  - Nickname and user registration (`NICK`, `USER`)  
  - Private messaging and channel messaging  
  - Channel creation, joining, and topic setting  
  - Server responses (`PING/PONG`, `ERROR`, welcome messages)  
- Handles multiple simultaneous clients using **epoll**  
- Graceful disconnection and signal handling  

## 🛠 Tech Stack

**Language & Libraries**
- C++ (C++98 standard)  
- POSIX socket programming  
- `epoll`, `poll`, and file descriptors  
- Signal handling (`SIGINT`, `SIGTERM`)  

**Concepts**
- TCP/IP socket communication  
- Non-blocking I/O  
- Event-driven architecture  
- Protocol design (IRC command parsing, reply formatting)  
- Concurrent client handling

## 📬 Contact

Built by [@lulubas](https://github.com/lulubas)
