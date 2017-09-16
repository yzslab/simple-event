#ifndef SIMPLEHTTP_LIBRARY_H
#define SIMPLEHTTP_LIBRARY_H

#define setServerSocketFD(simpleTCPServerInstance, fd) (simpleTCPServerInstance)->socketFD = (fd)
#define setClientSocketFD(simpleTCPServerClientInstance, fd) (simpleTCPServerClientInstance)->socketFD = (fd)
#define getServerSocketFD(simpleTCPServerInstance) (simpleTCPServerInstance)->socketFD
#define getClientSocketFD(simpleTCPServerClientInstance) (simpleTCPServerClientInstance)->socketFD
#include <netinet/in.h>

typedef struct simpleTCPServer {
    int socketFD;
} simpleTCPServer;

typedef struct simpleTCPServerClient {
    int socketFD;
    struct sockaddr_in clientAddr;
    socklen_t length;
} simpleTCPServerClient;


simpleTCPServer *createSimpleTCPServer(char *, int, int);
simpleTCPServerClient *simpleTCPServerAccept(simpleTCPServer *);
ssize_t simpleTCPServerReceive(simpleTCPServerClient *, void *, size_t);
ssize_t simpleTCPServerSend(simpleTCPServerClient *, void *, size_t );
void destroySimpleTCPServer(simpleTCPServer *simpleTCPServerInstance);
void destroySimpleTCPServerClient(simpleTCPServerClient *);

#endif