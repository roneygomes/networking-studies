#include <stdio.h>
#include <libc.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "3490"
#define BACKLOG 20

int get_addrinfo(struct addrinfo **host_addr) {
    struct addrinfo hints;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
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
    int reuse_socket = 1;

    struct addrinfo *i;

    for (i = host_addr; i != NULL; i = i->ai_next) {
        if ((socket_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1) {
            perror("failed to open socket");
            continue;
        }

        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_socket, sizeof(int)) == -1) {
            perror("failed to set socket options");
            return -1;
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

void sigchld_handler(int s) {
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

int reap_zombie_processes() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART;

    sigemptyset(&sa.sa_mask);

    return sigaction(SIGCHLD, &sa, NULL);
}

void server_loop(int socket_fd) {
    struct sockaddr_in remote_addr;
    socklen_t addr_size;

    int new_socket_fd;
    char ip_string[INET6_ADDRSTRLEN];

    for (;;) {
        addr_size = sizeof(remote_addr);
        // if the socket is blocking and the request queue is empty this call will block
        // until a new request arrives
        new_socket_fd = accept(socket_fd, (struct sockaddr *) &remote_addr, &addr_size);

        if (new_socket_fd == -1) {
            perror("failed to accept connection request from socket");
            continue;
        }

        inet_ntop(remote_addr.sin_family, &remote_addr.sin_addr, ip_string, sizeof(ip_string));
        printf("server: received connection from %s\n", ip_string);

        if (!fork()) {
            // this is a child process so the main listener socket is not needed
            close(socket_fd);

            if (send(new_socket_fd, "Hello World!", 13, 0) == -1) {
                perror("failed to send message to remote host");
            }
            // this is a child process so we don't continue looping to accept new
            // requests, we send the message and quit
            close(new_socket_fd);
            break;
        }

        // as the main process we have no use for the new socket
        close(new_socket_fd);
    }
}

int main(void) {
    struct addrinfo *host_addr = NULL;
    get_addrinfo(&host_addr);

    int socket_fd;
    if ((socket_fd = bind_socket(host_addr)) == -1) {
        return 1;
    }

    freeaddrinfo(host_addr);

    if (listen(socket_fd, BACKLOG) == -1) {
        perror("failed to listen to socket");
        return 1;
    }

    if (reap_zombie_processes() == -1) {
        perror("failed to reap zombie processes");
        return 1;
    }

    printf("server: waiting for connections... \n");
    server_loop(socket_fd);

    return 0;
}
