#pragma once
#include <winsock2.h>
#include <mswsock.h>
#include <cstdint>

constexpr size_t SEND_BUFFER_SIZE = 1024;
constexpr size_t RECV_BUFFER_SIZE = 1024;
constexpr size_t BUFSIZE = 1024;
constexpr size_t ACCEPT_BUFFER_SIZE = 2 * (sizeof(SOCKADDR_IN) + 16);

enum class OverlState :UINT16 {
    ACCEPT,
    RECV,
    SEND
};

struct stSendContext {
    WSAOVERLAPPED overlapped;
    WSABUF wsabuf;
    char buffer[SEND_BUFFER_SIZE];
    SOCKET sock;
    OverlState state;
};

struct stRecvContext {
    WSAOVERLAPPED overlapped;
    WSABUF wsabuf;
    char buffer[RECV_BUFFER_SIZE];
    SOCKET sock;
    OverlState state;
};

struct stAcceptContext {
    WSAOVERLAPPED overlapped;
    char buffer[ACCEPT_BUFFER_SIZE];
    SOCKET sock;
    OverlState state;
};
struct stOverlappedEx {
    WSAOVERLAPPED overlp;
    WSABUF wsabuf;
    char buf[BUFSIZE];
    SOCKET sock;
    OverlState state;
};