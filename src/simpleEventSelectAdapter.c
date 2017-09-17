//
// Created by zhensheng on 9/17/17.
//

#include <stdlib.h>
#include <sys/time.h>
#include "simpleEventSelectAdapter.h"

static simpleEventSelectAdapter *createSimpleEventSelectInstance(simpleEvent *simpleEventInstance, int maxEvents);
static int simpleEventSelectAddFD(simpleEventSelectAdapter *instance, simpleEventFD *fd);
static int simpleEventSelectRemoveFD(simpleEventSelectAdapter *instance, int fd);
static int simpleEventSelectWait(simpleEventSelectAdapter *instance, simpleEventFD *fds, int timeout);
static int simpleEventSelectClean(simpleEventSelectAdapter *instance);

simpleEventSelectAdapter *createSimpleEventSelectInstance(simpleEvent *simpleEventInstance, int maxEvents) {
    simpleEventSelectAdapter *instance = malloc(sizeof(simpleEventSelectAdapter));
    if (!instance)
        goto ON_ERROR;

    instance->simpleEventInstance = simpleEventInstance;
    instance->maxEvents = maxEvents;

    return instance;

    ON_ERROR:
    return NULL;
}

int simpleEventSelectAddFD(simpleEventSelectAdapter *instance, simpleEventFD *fd) {
    return 0;
}

int simpleEventSelectRemoveFD(simpleEventSelectAdapter *instance, int fd) {
    return 0;
}

int simpleEventSelectWait(simpleEventSelectAdapter *instance, simpleEventFD *fds, int timeout) {
    FD_ZERO(&instance->readFDs);
    FD_ZERO(&instance->writeFDs);
    FD_ZERO(&instance->exceptFDs);

    simpleEventIterator *iterator = simpleEventGetIterator(instance->simpleEventInstance);
    simpleEventFD *fd;
    while (simpleEventIteratorHasNext(instance->simpleEventInstance, iterator)) {
        simpleEventIteratorNext(instance->simpleEventInstance, iterator);
        fd = simpleEventIteratorGet(instance->simpleEventInstance, iterator);
        if (!fd)
            continue;
        if (fd->events & SIMPLE_EVENT_READ)
            FD_SET(fd->fd, &instance->readFDs);
        if (fd->events & SIMPLE_EVENT_WRITE)
            FD_SET(fd->fd, &instance->writeFDs);
        if (fd->events & SIMPLE_EVENT_EXCEPTION)
            FD_SET(fd->fd, &instance->exceptFDs);
        simpleEventIteratorValueClean(instance->simpleEventInstance, fd);
    }
    simpleEventCleanIterator(instance->simpleEventInstance, iterator);

    struct timeval tv, *tvPtr = &tv;
    suseconds_t microSeconds;
    if (timeout == -1)
        tvPtr = NULL;
    else {
        microSeconds = (timeout % 1000) * 1000;
        timeout /= 1000;
        tv.tv_sec = timeout;
        tv.tv_usec = microSeconds;
    }

    int happenEventNum = select(instance->maxEvents, &instance->readFDs, &instance->writeFDs, &instance->exceptFDs, tvPtr);
    if (happenEventNum < 0)
        return -1;
    else if (!happenEventNum)
        return 0;

    iterator = simpleEventGetIterator(instance->simpleEventInstance);
    int i = 0, event;
    while (simpleEventIteratorHasNext(instance->simpleEventInstance, iterator)) {
        simpleEventIteratorNext(instance->simpleEventInstance, iterator);
        fd = simpleEventIteratorGet(instance->simpleEventInstance, iterator);
        if (!fd)
            continue;

        event = 0;
        if (FD_ISSET(fd->fd, &instance->readFDs))
            event |= SIMPLE_EVENT_READ;
        if (FD_ISSET(fd->fd, &instance->writeFDs))
            event |= SIMPLE_EVENT_WRITE;
        if (FD_ISSET(fd->fd, &instance->exceptFDs))
            event |= SIMPLE_EVENT_EXCEPTION;
        if (!event)
            continue;

        fds[i] = *fd;
        fds[i].events &= event;
        simpleEventIteratorValueClean(instance->simpleEventInstance, fd);
        ++i;
    }
    simpleEventCleanIterator(instance->simpleEventInstance, iterator);

    return i;
}

int simpleEventSelectClean(simpleEventSelectAdapter *instance) {
    free(instance);
    return 0;
}

simpleEventHandler createSelectHandler() {
    simpleEventHandler handler;
    handler.handlerInit = &createSimpleEventSelectInstance;
    handler.handlerClean = &simpleEventSelectClean;
    handler.addFD = &simpleEventSelectAddFD;
    handler.removeFD = &simpleEventSelectRemoveFD;
    handler.handlerWait = &simpleEventSelectWait;

    return handler;
}