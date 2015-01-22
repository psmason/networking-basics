// Following "A Simple Stream Server" from Beej's guide to networking

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <iostream>

namespace {
    const char * const PORT = "3490";
    const int BACKLOG       = 10;

    void * getRawAddress(struct sockaddr_storage * addrinfo) {
        if (AF_INET == addrinfo->ss_family) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) addrinfo;
            return &(ipv4->sin_addr);
        }
        else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) addrinfo;
            return &(ipv6->sin6_addr);
        }
    }

    void printAddrInfo(addrinfo *servinfo) {
        struct addrinfo *p;
        char ipstr[INET6_ADDRSTRLEN];
        for (p=servinfo; p!=NULL; p=p->ai_next) {
            void *addr = getRawAddress((struct sockaddr_storage *)p->ai_addr);
            std::string ipver("");

            if (AF_INET == p->ai_family) {
                // struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                // addr = &(ipv4->sin_addr);
                ipver = "IPv4";
            }
            else {
                // struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                // addr = &(ipv6->sin6_addr);
                ipver = "IPv6";
            }
            
            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
            std::cout << "Server IP info (should be INADDR_ANY or IN6ADDR_ANY_INIT)... "
                      <<  ipver << ": " << ipstr << std::endl;
        }
    }
} // anonymous namespace

int main()
{
    int sockfd, newfd; // listen on sockfd, new connection on newfd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE; // use my IP

    //getaddrinfo(NULL, PORT, &hints, &servinfo);
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo) != 0)) {
        std::cout << "getaddrinfo error: " << gai_strerror(rv) << std::endl;
        return 1;
    }
    printAddrInfo(servinfo);

    for (p=servinfo; p!=NULL; p=p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cout << "Server socket error" << std::endl;
            continue;
        }

        if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes)) {
            std::cout << "Set socket option error" << std::endl;
            exit(1);
        }

        if (-1 == bind(sockfd, p->ai_addr, p->ai_addrlen)) {
            std::cout << "Bind error" << std::endl;
            continue;
        }

        break;
    }

    if (NULL == p) {
        std::cout << "Server failed to bind" << std::endl;
        return 2;
    }

    freeaddrinfo(servinfo); // cleanup

    if (-1 == listen(sockfd, BACKLOG)) {
        std::cout << "Failed to listen" << std::endl;
        exit(1);
    }

    std::cout << "Server waiting for connections..." << std::endl;

    while (true) { // main accept() loop
        sin_size = sizeof their_addr;
        newfd    = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (-1 == newfd) {
            std::cout << "Failed to accept connection" << std::endl;
            continue;
        }

        inet_ntop(their_addr.ss_family
                  , getRawAddress(&their_addr)
                  , s, sizeof s);
        std::cout << "Server connection received from " << s << std::endl;

        if (~fork()) {
            close(sockfd); // child does not need the listener
            if (-1 == send(newfd, "Hello, world!", 13, 0)) {
                std::cout << "Failed to send hello, world response" << std::endl;
            }
            close(newfd);
            exit(0);
        }
        close(newfd);
    }

    return 0;
}


















