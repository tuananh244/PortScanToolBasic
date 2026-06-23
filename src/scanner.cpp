#include "scanner.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

bool check_port(const std::string& ip, int port, int timeout_ms) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return false;

    // Chuyển socket sang chế độ Non-blocking
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &target.sin_addr);

    // Bắt đầu kết nối (sẽ trả về WSAEWOULDBLOCK ngay lập tức)
    connect(sock, (sockaddr*)&target, sizeof(target));

    fd_set fdWrite, fdErr;
    FD_ZERO(&fdWrite);
    FD_ZERO(&fdErr);
    FD_SET(sock, &fdWrite);
    FD_SET(sock, &fdErr);

    TIMEVAL tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    bool is_open = false;
    
    // Đợi kết nối hoặc báo lỗi trong khoảng timeout
    int result = select(0, NULL, &fdWrite, &fdErr, &tv);
    
    if (result > 0 && FD_ISSET(sock, &fdWrite) && !FD_ISSET(sock, &fdErr)) {
        is_open = true; // Socket có thể ghi và không có lỗi -> Port mở
    }

    closesocket(sock);
    return is_open;
}