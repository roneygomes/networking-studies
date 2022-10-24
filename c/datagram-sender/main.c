#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>
#include <libc.h>

#define PORT "3490"

int get_addrinfo(char *hostname, struct addrinfo **host_addr) {
    struct addrinfo hints;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(hostname, PORT, &hints, host_addr) != 0) {
        perror("failed to resolve remote address");
        return -1;
    }

    return 0;
}

int open_socket(struct addrinfo *host_addr) {
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

        break;
    }

    if (i == NULL) {
        fprintf(stderr, "failed to open socket\n");
        return -1;
    }

    return socket_fd;
}

ssize_t write_to_socket(int socket_fd, char *message, struct addrinfo *host_addr) {
    ssize_t num_bytes;
    int flags = 0;

    num_bytes = sendto(socket_fd, message, strlen(message), flags, host_addr->ai_addr, host_addr->ai_addrlen);

    if (num_bytes == -1) {
        perror("failed to write to socket");
        return -1;
    }

    return num_bytes;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: sender hostname message\n");
        return 1;
    }

    struct addrinfo *remote_addr = NULL;
    get_addrinfo(argv[1], &remote_addr);

    int socket_fd;
    if ((socket_fd = open_socket(remote_addr)) == -1) {
        return 1;
    }

    ssize_t num_bytes;
    if ((num_bytes = write_to_socket(socket_fd, argv[2], remote_addr)) == -1) {
        return 1;
    }

    freeaddrinfo(remote_addr);

    printf("sent %zd bytes to to %s\n", num_bytes, argv[1]);
    close(socket_fd);

    return 0;
}
