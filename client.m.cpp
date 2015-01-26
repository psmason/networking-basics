#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <iostream>

#define PORT "3490"
#define MAXDATASIZE 100

namespace {
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
} // anonymous

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (2 != argc) {
        std::cout << "Client hostname argument required" << std::endl;
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        std::cout << "Failed to get address info: "
                  << gai_strerror(rv)
                  << std::endl;
        exit(1);
    }

    for (p=servinfo; p!=NULL; p=p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cout << "Failed to create socket..." << std::endl;
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::cout << "Failed to connect to socket..." << std::endl;
            continue;
        }

        break;
    }

    if (NULL == p) {
        std::cout << "Client failed to connect" << std::endl;
        exit(2);
    }

    inet_ntop(p->ai_family, getRawAddress((struct sockaddr_storage *)p->ai_addr), s, sizeof s);
    std::cout << "Client connected to " << s << std::endl;

    freeaddrinfo(servinfo);
    
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        std::cout << "Client failed to receive data" << std::endl;
        exit(1);
    }

    buf[numbytes] = '\0';
    std::cout << "client received: " << buf << std::endl;
    close(sockfd);

    return 0;
}





