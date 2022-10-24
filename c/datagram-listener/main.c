#include <stdio.h>
#include <libc.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "3490"
#define BUFFER_LENGTH 100

int get_addrinfo(struct addrinfo **host_addr) {
    struct addrinfo hints;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, host_addr) != 0) {
        perror("failed to resolve remote address");
        return -1;
    }

    return 0;
}

int bind_socket(struct addrinfo *host_addr) {
    if (host_addr == NULL) {
        fprintf(stderr, "failed to open socket: uninitialized addrinfo structure\n");
        return -1;
    }

    int socket_fd;
    struct addrinfo *i;

    for (i = host_addr; i != NULL; i = i->ai_next) {
        if ((socket_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
            perror("failed to open socket");
            continue;
        }

        if (bind(socket_fd, i->ai_addr, i->ai_addrlen) == -1) {
            perror("failed to bind socket");
            close(socket_fd);
            return -1;
        }

        break;
    }

    if (i == NULL) {
        fprintf(stderr, "failed to bind socket\n");
        return -1;
    }

    return socket_fd;
}

ssize_t read_from_socket(int socket_fd, void *buffer) {
    struct sockaddr_in6 remote_addr;
    socklen_t addr_size = sizeof(remote_addr);

    int flags = 0;

    size_t read_length = BUFFER_LENGTH - 1;
    ssize_t num_bytes;

    num_bytes = recvfrom(socket_fd, buffer, read_length, flags, (struct sockaddr *) &remote_addr, &addr_size);

    if (num_bytes == -1) {
        perror("failed to read from socket");
        return -1;
    }

    char ip_string[INET6_ADDRSTRLEN];
    inet_ntop(remote_addr.sin6_family, &remote_addr.sin6_addr, ip_string, sizeof(ip_string));
    printf("got packet from %s\n", ip_string);

    return num_bytes;
}

int main(void) {
    struct addrinfo *host_addr = NULL;
    get_addrinfo(&host_addr);

    int socket_fd;
    if ((socket_fd = bind_socket(host_addr)) == -1) {
        return 1;
    }

    freeaddrinfo(host_addr);

    char buffer[BUFFER_LENGTH];
    ssize_t num_bytes;

    if ((num_bytes = read_from_socket(socket_fd, &buffer)) == -1) {
        return 1;
    }

    printf("packet is %zd bytes long\n", num_bytes);
    buffer[num_bytes] = '\0';

    printf("packet contains \"%s\"\n", buffer);
    close(socket_fd);

    return 0;
}
