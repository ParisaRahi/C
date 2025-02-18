/*
 * This is a file that implements the operation on TCP sockets that are used by
 * all of the programs used in this assignment.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "connection.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tcp_connect(char *hostname, int port)
{
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Could not create socket");
        return -1;
    }

    server.sin_addr.s_addr = inet_addr(hostname);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        fprintf(stderr, "Connection failed\n");
        return -1;
    }

    return sock;
}

int tcp_read(int sock, char *buffer, int n)
{
    int bytes_read;

    bytes_read = read(sock, buffer, n);
    if (bytes_read < 0)
    {
        fprintf(stderr,"Read failed\n");
        return -1;
    }

    return bytes_read;
}

int tcp_write(int sock, char *buffer, int bytes)
{
    int bytes_written;

    bytes_written = write(sock, buffer, bytes);
    if (bytes_written < 0)
    {
        fprintf(stderr,"Write failed\n");
        return -1;
    }

    return bytes_written;
}

int tcp_write_loop(int sock, char *buffer, int bytes)
{
    int bytes_left = bytes;
    int bytes_written = 0;

    while (bytes_left > 0)
    {
        int result = tcp_write(sock, buffer + bytes_written, bytes_left);
        if (result < 0)
            return -1;
        bytes_written += result;
        bytes_left -= result;
    }

    return bytes_written;
}

void tcp_close(int sock)
{
    close(sock);
}

int tcp_create_and_listen(int port)
{
    int server_sock;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sock == -1)
    {
        fprintf(stderr,"Failed to create socket\n");
        exit(-1);
    }
    
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to bind socket\n");
        exit(-1);
    }

    // Listen on server socket
    if (listen(server_sock, 100) == -1)
    {
        perror("Failed to listen on socket");
        exit(-1);
    }

    //printf("Server listening on port %d\n", port);
    return server_sock;
}

int tcp_accept(int server_sock)
{
    int client_sock;
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);

    client_sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&c);
    if (client_sock < 0)
    {
        fprintf(stderr,"Accept failed\n");
        return -1;
    }

    return client_sock;
}

int tcp_wait(fd_set *waiting_set, int wait_end)
{
    int activity;

    activity = select(wait_end + 1, waiting_set, NULL, NULL, NULL);

    if (activity < 0)
    {
        perror("Select failed");
        return -1;
    }

    return activity;
}

int tcp_wait_timeout(fd_set *waiting_set, int wait_end, int seconds)
{
    int activity;
    struct timeval tv;

    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    activity = select(wait_end + 1, waiting_set, NULL, NULL, &tv);

    if (activity < 0)
    {
        perror("Select failed");
        return -1;
    }

    return activity;
}
