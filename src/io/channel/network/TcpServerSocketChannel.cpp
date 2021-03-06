//
// Created by anarion on 2020/2/22.
//

#include <arpa/inet.h>
#include "io/channel/network/TcpServerSocketChannel.h"

void anarion::TcpServerSocketChannel::bind(in_port_t port_num) {
    if (port_num == local_addr.sin_port) { return; }
    local_addr.sin_port = htons(port_num);
    int ret = ::bind(sockfd, reinterpret_cast<const sockaddr *>(&local_addr), sizeof(sockaddr_in));
    if (ret < 0) {

    }
}

void anarion::TcpServerSocketChannel::listen(int backlog) {
    ::listen(sockfd, backlog);
}

int anarion::TcpServerSocketChannel::accept() {
    int cfd;
//    while (true) {
//        cfd = ::accept(sockfd, nullptr, nullptr);
//        if (cfd < 0) {
//            if (errno == ECONNABORTED) {
//
//            }
//        }
//    }
    cfd = ::accept(sockfd, nullptr, nullptr);
    if (cfd < 0) {
        throwSocket();
    }
    return cfd;
}

anarion::TcpServerSocketChannel::TcpServerSocketChannel(in_port_t port_num) : InChannel(true), OutChannel(true), SocketChannel(0), TcpSocketChannel(), ServerSocketChannel(port_num) {
    ::memset(&local_addr, 0, sizeof(sockaddr_in));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(port_num);
    int ret = ::bind(sockfd, reinterpret_cast<const sockaddr *>(&local_addr), sizeof(sockaddr_in));
    if (ret < 0) { throwSocket(); }
}

anarion::HostInfo anarion::TcpServerSocketChannel::acceptWithInfo() {
    socklen_t len = sizeof(sockaddr_in);
    sockaddr_in client_addr;
    int cfd = ::accept(sockfd, reinterpret_cast<sockaddr *>(&client_addr), &len);
    if (cfd < 0) {
        throwSocket();
    }
    HostInfo info;
    inet_ntop(AF_INET, &client_addr.sin_addr, info.ip_str, INET_ADDRSTRLEN);
    info.portNum = ntohs(client_addr.sin_port);
    return info;
}

