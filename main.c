//
// Created by zhensheng on 9/14/17.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "src/hashTable.h"
#include "src/simpleEvent.h"
#include "src/simpleEventEpollAdapter.h"
#include "src/simpleEventSelectAdapter.h"
#include "src/simpleEventPollAdapter.h"
#include "src/simpleEventHashTableAdapter.h"
#include "src/simpleTCPServer.h"

#define TEST_SIMPLE_TCP_SERVER
#define TEST_SIMPLE_EVENT
#define TEST_EPOLL_ADAPTER
// #define TEST_SELECT_ADAPTER
// #define TEST_POLL_ADAPTER

int main(int argc, char *argv[]) {
#ifdef TEST_HASH_TABLE
    hashTable *hashTableInstance1 = createHashTableInstance(10, sizeof(int), sizeof(double), NULL);
    int i = 0;
    double tmp;
    for (i = 0, tmp = 0; i < 10; ++i) {
        ++tmp;
        hashTablePut(hashTableInstance1, &i, &tmp);
    }
    for (i = 9; i >= 0; --i)
        printf("%lf\n", * (double *) hashTableGet(hashTableInstance1, &i, &tmp));
    for (i = 0; i < 10; ++i, ++i)
        hashTableRemove(hashTableInstance1, &i);
    putchar('\n');
    for (i = 0; i < 10; ++i) {
        if (hashTableGet(hashTableInstance1, &i, &tmp))
            printf("%lf\n", tmp);
    }
    putchar('\n');
    hashTableIterator *iterator;
    iterator = hashTableGetIterator(hashTableInstance1);
    while (hashTableIteratorHasNext(iterator)) {
        hashTableIteratorNext(iterator);
        printf("%d: %lf\n", *(int *) hashTableIteratorKey(iterator), *(double *) hashTableIteratorValue(iterator));
        ++tmp;
        hashTableIteratorSet(iterator, &tmp);
    }
    hashTableCleanIterator(iterator);
    putchar('\n');
    iterator = hashTableGetIterator(hashTableInstance1);
    while (hashTableIteratorHasNext(iterator)) {
        hashTableIteratorNext(iterator);
        printf("%d: %lf\n", *(int *) hashTableIteratorKey(iterator), *(double *) hashTableIteratorValue(iterator));
        if (*(int *) hashTableIteratorKey(iterator) == 7)
            hashTableIteratorRemove(iterator);
    }
    hashTableCleanIterator(iterator);
    putchar('\n');
    iterator = hashTableGetIterator(hashTableInstance1);
    while (hashTableIteratorHasNext(iterator)) {
        hashTableIteratorNext(iterator);
        printf("%d: %lf\n", *(int *) hashTableIteratorKey(iterator), *(double *) hashTableIteratorValue(iterator));
    }
    hashTableCleanIterator(iterator);
    cleanHashTableInstance(hashTableInstance1);
#endif

#ifdef TEST_SIMPLE_TCP_SERVER
    char *sendText = "Welcome!\n";
    size_t sendTextLength = strlen(sendText);

    const int bufferSize = 256;
    char buffer[bufferSize];

    simpleTCPServer *simpleTCPServerInstance = createSimpleTCPServer("0.0.0.0", 8088, 512);
    if (!simpleTCPServerInstance)
        goto PERROR_THEN_EXIT;

#ifdef TEST_SIMPLE_EVENT
    simpleEventHandler eventHandler =
#ifdef TEST_EPOLL_ADAPTER
            createEpollHandler()
#endif
#ifdef TEST_SELECT_ADAPTER
            createSelectHandler()
#endif
#ifdef TEST_POLL_ADAPTER
            createPollHandler()
#endif
    ;
    simpleEventContainerHandler containerHandler = createHashTableHandler();
    simpleEvent *simpleEventInstance = createSimpleEvent(100, &eventHandler, &containerHandler);

    simpleEventAddFD(simpleEventInstance, getServerSocketFD(simpleTCPServerInstance), SIMPLE_EVENT_READ, NULL);
    simpleEventFD *fds;
    int eventNum, i;
    simpleTCPServerClient *simpleTCPServerClientInstance;
    ssize_t receiveLength, sendLength, totalReceiveBytes;
    void *ptr, *content;
    while (1) {
        eventNum = simpleEventWait(simpleEventInstance, &fds, -1);
        printf("%d events.\n", eventNum);
        for (i = 0; i < eventNum; ++i) {
            if (fds[i].data == NULL) {
                while (simpleTCPServerClientInstance = simpleTCPServerAccept(simpleTCPServerInstance)) {
                    simpleEventAddFD(simpleEventInstance, getClientSocketFD(simpleTCPServerClientInstance),
                                     SIMPLE_EVENT_READ, simpleTCPServerClientInstance);
                    // simpleTCPServerSend(simpleTCPServerClientInstance, sendText, sendTextLength);
                }
            } else {
                simpleTCPServerClientInstance = fds[i].data;
                totalReceiveBytes = 0;
                while ((receiveLength = simpleTCPServerReceive(simpleTCPServerClientInstance, &buffer, bufferSize)) > 0) {
                    totalReceiveBytes += receiveLength;
                    // printf("%d bytes received.\n", receiveLength);
                    write(STDOUT_FILENO, buffer, receiveLength);
                    // sendLength = simpleTCPServerSend(simpleTCPServerClientInstance, sendText, sendTextLength);
                    // printf("%d bytes sent.\n", sendLength);
                }
                if (receiveLength == 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    simpleEventRemoveFD(simpleEventInstance, getClientSocketFD(simpleTCPServerClientInstance));
                    destroySimpleTCPServerClient(simpleTCPServerClientInstance);
                } else {
                    content = "<h1>Welcome!</h1>";
                    printf("\nRead finished, total %zd bytes.\n", totalReceiveBytes);
                    sprintf(buffer, "%zd", strlen(content));
                    ptr = "HTTP/1.1 200 OK\r\nServer: Simple TCP Server\r\nContent-Length:";
                    simpleTCPServerSend(simpleTCPServerClientInstance, ptr, strlen(ptr));
                    simpleTCPServerSend(simpleTCPServerClientInstance, buffer, strlen(buffer));
                    ptr = "\r\n\r\n";
                    simpleTCPServerSend(simpleTCPServerClientInstance, ptr, strlen(ptr));
                    simpleTCPServerSend(simpleTCPServerClientInstance, content, strlen(content));
                }
            }
        }
        simpleEventCleanEvents(fds);
    }
#else
    simpleTCPServerClient *simpleTCPServerClientInstance = simpleTCPServerAccept(simpleTCPServerInstance);
    simpleTCPServerSend(simpleTCPServerClientInstance, sendText, sendTextLength);

    sendText = "Received!\n";
    sendTextLength = strlen(sendText);
    ssize_t receiveLength, sendLength;
    while ((receiveLength = simpleTCPServerReceive(simpleTCPServerClientInstance, &buffer, bufferSize)) > 0) {
        printf("%d bytes received.\n", receiveLength);
        write(STDOUT_FILENO, buffer, receiveLength);
        sendLength = simpleTCPServerSend(simpleTCPServerClientInstance, sendText, sendTextLength);
        printf("%d bytes sent.\n", sendLength);
    }
#endif
#endif
    return 0;

    PERROR_THEN_EXIT:
    perror("error");
    return 1;
}