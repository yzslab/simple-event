#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "simpleEvent.h"

simpleEvent *createSimpleEvent(unsigned int maxEvents, simpleEventHandler *eventHandler, simpleEventContainerHandler *containerHandler) {
    simpleEvent *instance = malloc(sizeof(simpleEvent) + sizeof(simpleEventFD) * maxEvents);
    if (!instance)
        return NULL;

    instance->maxEvents = maxEvents;

    /*
    memcpy(&instance->eventHandler, eventHandler, sizeof(simpleEventHandler));
    memcpy(&instance->containerHandler, containerHandler, sizeof(containerHandler));
    */

    instance->eventHandler = *eventHandler;
    instance->containerHandler = *containerHandler;

    instance->eventInstance = instance->eventHandler.handlerInit(instance, maxEvents);
    instance->containerInstance = instance->containerHandler.handlerInit(maxEvents);

    return instance;
}

void destroySimpleEvent(simpleEvent *instance) {
    instance->containerHandler.clean(instance->containerInstance);
    instance->eventHandler.handlerClean(instance->eventInstance);
    free(instance);
}

int simpleEventAddFD(simpleEvent *instance, int fd, simpleEventList events, void *data) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    simpleEventFD fdInstance;
    fdInstance.fd = fd;
    fdInstance.events = events;
    fdInstance.data = data;

    if (instance->containerHandler.put(instance->containerInstance, &fdInstance))
        goto ON_ERROR;

    if (instance->eventHandler.addFD(instance->eventInstance, &fdInstance))
        goto REMOVE_FROM_CONTAINER_ON_ERROR;

    return 0;

    REMOVE_FROM_CONTAINER_ON_ERROR:
    instance->containerHandler.remove(instance->containerInstance, fd);
    ON_ERROR:
    return -1;
}

int simpleEventRemoveFD(simpleEvent *instance, int fd) {
    simpleEventFD *backupFD;

    if (!(backupFD = instance->containerHandler.get(instance->containerInstance, fd)))
        return -1;

    if (instance->eventHandler.removeFD(instance->eventInstance, fd))
        goto FREE_ON_ERROR;

    if (instance->containerHandler.remove(instance->containerInstance, fd))
        goto PUT_BACK_TO_EVENT_HANDLER_ON_ERROR;

    instance->containerHandler.clean(backupFD);
    return 0;

    PUT_BACK_TO_EVENT_HANDLER_ON_ERROR:
    instance->eventHandler.addFD(instance->containerInstance, backupFD);
    FREE_ON_ERROR:
    instance->containerHandler.clean(backupFD);
    return -1;
}

int simpleEventWait(simpleEvent *instance, simpleEventFD **fds, int timeout) {
    *fds = malloc(sizeof(simpleEventFD) * instance->maxEvents);
    if (!*fds)
        return -1;
    return instance->eventHandler.handlerWait(instance->eventInstance, *fds, timeout);
}

int simpleEventCleanEvents(simpleEventFD *fds) {
    free(fds);
}

int simpleEventWaitCallback(simpleEvent *instance, int timeout, simpleEventCallbackFunction callback) {
    simpleEventFD *fds;
    int happenEventNum = simpleEventWait(instance, &fds, timeout);
    int i;
    for (i = 0; i < happenEventNum; ++i)
        callback(&fds[i]);

    simpleEventCleanEvents(fds);

    return happenEventNum;
}

simpleEventFD *simpleEventGetData(simpleEvent *instance, int fd) {
    return instance->containerHandler.get(instance->containerInstance, fd);
}

int simpleEventDataClean(simpleEvent *instance, simpleEventFD *fd) {
    return instance->containerHandler.clean(fd);
}

void *simpleEventGetIterator(simpleEvent *instance) {
    return instance->containerHandler.getIterator(instance->containerInstance);
}

int simpleEventIteratorHasNext(simpleEvent *instance, void *iterator) {
    return instance->containerHandler.iteratorHasNext(iterator);
}

int simpleEventIteratorNext(simpleEvent *instance, void *iterator) {
    return instance->containerHandler.iteratorNext(iterator);
}

simpleEventFD *simpleEventIteratorGet(simpleEvent *instance, void *iterator) {
    return instance->containerHandler.iteratorGet(iterator);
}

int simpleEventIteratorValueClean(simpleEvent *instance, void *iterator) {
    return instance->containerHandler.iteratorValueClean(iterator);
}

int simpleEventIteratorSet(simpleEvent *instance, void *iterator, void *value) {
    return instance->containerHandler.iteratorSet(iterator, value);
}

int simpleEventCleanIterator(simpleEvent *instance, void *iterator) {
    return instance->containerHandler.cleanIterator(iterator);
}