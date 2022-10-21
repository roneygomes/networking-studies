#include <sys/socket.h>
#include <netdb.h>
#include <memory.h>
#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }

    struct addrinfo hints;
    struct addrinfo *service_info;

    memset(&hints, 0, sizeof hints);    // make sure the struct is empty

    hints.ai_family = AF_UNSPEC;        // can be either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP stream stockets

    int status;

    if ((status = getaddrinfo(argv[1], NULL, &hints, &service_info)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    char ip_string[INET6_ADDRSTRLEN];

    for (struct addrinfo *ai = service_info; ai != NULL; ai = ai->ai_next) {
        void *address;
        char *ip_version;

        if (ai->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) ai->ai_addr;
            address = &(ipv4->sin_addr);
            ip_version = "IPv4";
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) ai->ai_addr;
            address = &(ipv6->sin6_addr);
            ip_version = "IPv6";
        }

        inet_ntop(ai->ai_family, address, ip_string, sizeof ip_string);
        printf("  %s: %s\n", ip_version, ip_string);
    }

    freeaddrinfo(service_info);        // free the linked-list
    return 0;
}
