#include "simpleTCPServer.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

// inline static int getServerSocketFD(simpleTCPServer *simpleHTTPInstance);
// inline static int getClientSocketFD(simpleTCPServerClient *simpleTCPServerClientInstance);
inline static void closeSimpleTCPConnection(simpleTCPServerClient *simpleHTTPClientInstance);

simpleTCPServer *createSimpleTCPServer(char *listenAddr, int port, int backlog) {
    // Create simpleTCPServer instance
    simpleTCPServer *simpleTCPServerInstance = malloc(sizeof(simpleTCPServer));
    // Created successfully?
    if (!simpleTCPServerInstance)
        return NULL;

    // Create socket
    setServerSocketFD(simpleTCPServerInstance, socket(AF_INET, SOCK_STREAM, 0));
    // simpleTCPServerInstance->socketFD = socket(AF_INET, SOCK_STREAM, 0);
    // Socket created successfully?
    if (getServerSocketFD(simpleTCPServerInstance) < 0)
        goto FREE_ON_ERROR;

    int optval = 1;
    setsockopt(getServerSocketFD(simpleTCPServerInstance), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    struct in_addr binaryListenAddr;
    if (!listenAddr)
        binaryListenAddr.s_addr = htonl(INADDR_ANY);
    else if (inet_pton(AF_INET, listenAddr, &binaryListenAddr) == -1)
        goto CLOSE_SOCKET_ON_ERROR;
    serverAddr.sin_addr.s_addr = binaryListenAddr.s_addr;
    serverAddr.sin_port = htons((unsigned short) port);

    if (bind(getServerSocketFD(simpleTCPServerInstance), (struct sockaddr *) &serverAddr, sizeof(struct sockaddr_in)) < 0)
        goto CLOSE_SOCKET_ON_ERROR;

    if (listen(getServerSocketFD(simpleTCPServerInstance), backlog) < 0)
        goto CLOSE_SOCKET_ON_ERROR;

    return simpleTCPServerInstance;

    CLOSE_SOCKET_ON_ERROR:
    close(getServerSocketFD(simpleTCPServerInstance));
    FREE_ON_ERROR:
    free(simpleTCPServerInstance);
    return NULL;
}

simpleTCPServerClient *simpleTCPServerAccept(simpleTCPServer *simpleHTTPInstance) {
    // Create simpleTCPServerClient instance
    simpleTCPServerClient *simpleTCPServerClientInstance = malloc(sizeof(simpleTCPServerClient));
    // Created successfully?
    if (!simpleTCPServerClientInstance)
        return NULL;

    if ((setClientSocketFD(simpleTCPServerClientInstance, accept(getServerSocketFD(simpleHTTPInstance), (struct sockaddr *) &simpleTCPServerClientInstance->clientAddr, &simpleTCPServerClientInstance->length))) < 0)
        goto FREE_ON_ERROR;

    return simpleTCPServerClientInstance;

    FREE_ON_ERROR:
    free(simpleTCPServerClientInstance);
    return NULL;
}

ssize_t simpleTCPServerReceive(simpleTCPServerClient *simpleTCPServerClientInstance, void *buffer, size_t bufferSize) {
    return read(getClientSocketFD(simpleTCPServerClientInstance), buffer, bufferSize);
}

ssize_t simpleTCPServerSend(simpleTCPServerClient *simpleTCPServerClientInstance, void *buffer, size_t bufferSize) {
    return write(getClientSocketFD(simpleTCPServerClientInstance), buffer, bufferSize);
}

void destroySimpleTCPServer(simpleTCPServer *simpleTCPServerInstance) {
    close(getServerSocketFD(simpleTCPServerInstance));
    free(simpleTCPServerInstance);
}

void destroySimpleTCPServerClient(simpleTCPServerClient *simpleTCPServerClientInstance) {
    closeSimpleTCPConnection(simpleTCPServerClientInstance);
    free(simpleTCPServerClientInstance);
}

void closeSimpleTCPConnection(simpleTCPServerClient *simpleHTTPClientInstance) {
    close(simpleHTTPClientInstance->socketFD);
}

/*
int getServerSocketFD(simpleTCPServer *simpleHTTPInstance) {
    return simpleHTTPInstance->socketFD;
}

int getClientSocketFD(simpleTCPServerClient *simpleTCPServerClientInstance) {
    return simpleTCPServerClientInstance->socketFD;
}
*/