#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>
#include <libc.h>

#define PORT "3490"
#define MAX_BYTES 100

void *get_internet_addr(struct sockaddr *host_addr) {
    if (host_addr->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) host_addr)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) host_addr)->sin6_addr);
}

int get_addrinfo(char *hostname, struct addrinfo **host_addr) {
    struct addrinfo hints;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, PORT, &hints, host_addr) != 0) {
        perror("failed to resolver remote address");
        return -1;
    }

    return 0;
}

int connect_to(struct addrinfo *host_addr) {
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

        if (connect(socket_fd, i->ai_addr, i->ai_addrlen) == -1) {
            close(socket_fd);
            perror("failed to connect to socket");
            continue;
        }

        break;
    }

    if (i == NULL) {
        fprintf(stderr, "failed to connect\n");
        return -1;
    }

    char ip_string[INET6_ADDRSTRLEN];
    void *in_addr = get_internet_addr((struct sockaddr *) i->ai_addr);

    inet_ntop(i->ai_family, in_addr, ip_string, sizeof(ip_string));
    printf("connecting to host %s\n", ip_string);

    return socket_fd;
}

int read_socket(int socket_fd) {
    ssize_t num_bytes;
    char buffer[MAX_BYTES];

    if ((num_bytes = recv(socket_fd, buffer, MAX_BYTES - 1, 0)) == -1) {
        perror("failed to read from socket");
        return -1;
    }

    buffer[num_bytes] = '\0';
    printf("received: '%s'\n", buffer);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        return 1;
    }

    struct addrinfo *remote_addr = NULL;
    get_addrinfo(argv[1], &remote_addr);

    int socket_fd;
    if ((socket_fd = connect_to(remote_addr)) == -1) {
        return 1;
    }

    if (read_socket(socket_fd) == -1) {
        return 1;
    }

    freeaddrinfo(remote_addr);
    close(socket_fd);

    return 0;
}
