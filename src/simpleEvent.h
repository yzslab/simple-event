#ifndef SIMPLEEVENT_LIBRARY_H
#define SIMPLEEVENT_LIBRARY_H

#include <pthread.h>

typedef struct simpleEvent simpleEvent;
typedef void simpleEventIterator;

typedef enum simpleEventType {
    SIMPLE_EVENT_TYPE_EPOLL,
    SIMPLE_EVENT_TYPE_POLL,
    SIMPLE_EVENT_TYPE_SELECT
} simpleEventType;

typedef enum simpleEventList {
    SIMPLE_EVENT_READ = 1,
    SIMPLE_EVENT_WRITE = 1 << 1,
    SIMPLE_EVENT_EXCEPTION = 1 << 2,
} simpleEventList;

typedef struct simpleEventFD {
    int fd;
    simpleEventList events;
    void *data;
} simpleEventFD;

// I/O event callback function
typedef void *(*simpleEventCallbackFunction)(const simpleEventFD *);
// I/O monitor interface
typedef struct simpleEventHandler {
    void *(*handlerInit)(simpleEvent *, int capacity);
    int (*handlerClean)(void *instance);
    int (*addFD)(void *instance, simpleEventFD *fd); // Add fd to I/O monitor
    int (*removeFD)(void *instance, int fd); // Remove fd from I/O monitor
    int (*handlerWait)(void *adapterInstance, simpleEventFD *fds, int timeout); // Start I/O monitor, call callback() after I/O monitor return
} simpleEventHandler;

// Container interface
typedef struct simpleEventContainerHandler {
    void *(*handlerInit)(unsigned int capacity); // Create container instance
    int (*handleClean)(void *instance); // Clean container instance
    int (*put)(void *instance, simpleEventFD *fd); // Put a simpleEventFD into container instance
    simpleEventFD *(*get)(void *instance, int fd); // Get a simpleEventFD from container
    int (*clean)(simpleEventFD *fd); // Clean the value return from get()
    int (*set)(void *instance, int fd, simpleEventFD *newFD); // Update the value in container
    int (*remove)(void *instance, int fd); // Remove a fd from container
    simpleEventIterator *(*getIterator)(void *instance); // Get an iterator
    int (*iteratorHasNext)(void *iterator); // Know whether iterator has been pointed to the last fd
    int (*iteratorNext)(void *iterator); // Move iterator forward
    simpleEventFD *(*iteratorGet)(void *iterator); // Get value from container via iterator
    int (*iteratorValueClean)(simpleEventFD *fd); // Clean the value return from iteratorGet()
    int (*iteratorSet)(void *iterator, simpleEventFD *newFD); // Update value via iterator
    int (*cleanIterator)(void *iterator); // Clean iterator
} simpleEventContainerHandler;

struct simpleEvent {
    int maxEvents;
    simpleEventHandler eventHandler;
    simpleEventContainerHandler containerHandler;
    void *eventInstance;
    void *containerInstance;
};

simpleEvent *createSimpleEvent(unsigned int maxEvents, simpleEventHandler *eventHandler, simpleEventContainerHandler *containerHandler);
int simpleEventAddFD(simpleEvent *instance, int fd, simpleEventList events, void *data);
int simpleEventRemoveFD(simpleEvent *instance, int fd);
int simpleEventWait(simpleEvent *instance, simpleEventFD **fds, int timeout);
int simpleEventCleanEvents(simpleEventFD *fds);
int simpleEventWaitCallback(simpleEvent *instance, int timeout, simpleEventCallbackFunction callback);
simpleEventFD *simpleEventGetData(simpleEvent *instance, int fd);
int simpleEventDataClean(simpleEvent *instance, simpleEventFD *fd);

void *simpleEventGetIterator(simpleEvent *instance);
int simpleEventIteratorHasNext(simpleEvent *instance, void *iterator);
int simpleEventIteratorNext(simpleEvent *instance, void *iterator);
simpleEventFD *simpleEventIteratorGet(simpleEvent *instance, void *iterator);
int simpleEventIteratorValueClean(simpleEvent *instance, void *iterator);
int simpleEventIteratorSet(simpleEvent *instance, void *iterator, void *value);
int simpleEventCleanIterator(simpleEvent *instance, void *iterator);

#endif