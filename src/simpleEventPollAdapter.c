//
// Created by zhensheng on 9/17/17.
//
#include <stdlib.h>
#include "simpleEvent.h"
#include "simpleEventPollAdapter.h"

static simpleEventPollAdapter *createsimpleEventPollInstance(simpleEvent *simpleEventInstance, int maxEvents);
static int simpleEventPollAddFD(simpleEventPollAdapter *, simpleEventFD *);
static int simpleEventPollRemoveFD(simpleEventPollAdapter *, int);
static int simpleEventPollWait(simpleEventPollAdapter *instance, simpleEventFD *fds, int timeout);
static int simpleEventPollClean(simpleEventPollAdapter *);

simpleEventPollAdapter *createsimpleEventPollInstance(simpleEvent *simpleEventInstance, int maxEvents) {
    simpleEventPollAdapter *instance = malloc(sizeof(simpleEventPollAdapter) + sizeof(struct pollfd) * maxEvents);
    if (!instance)
        goto ON_ERROR;

    instance->simpleEventInstance = simpleEventInstance;
    instance->maxEvents = maxEvents;
    instance->modified = 0;

    return instance;

    ON_ERROR:
    return NULL;
}

int simpleEventPollAddFD(simpleEventPollAdapter *instance, simpleEventFD *fd) {
    instance->modified = 1;
    return 0;
}

int simpleEventPollRemoveFD(simpleEventPollAdapter *instance, int fd) {
    instance->modified = 1;
    return 0;
}

int simpleEventPollWait(simpleEventPollAdapter *instance, simpleEventFD *fds, int timeout) {
    simpleEventIterator *iterator;
    simpleEventFD *fd;
    int i = 0;
    if (instance->modified) {
        iterator = simpleEventGetIterator(instance->simpleEventInstance);
        while (simpleEventIteratorHasNext(instance->simpleEventInstance, iterator)) {
            simpleEventIteratorNext(instance->simpleEventInstance, iterator);
            fd = simpleEventIteratorGet(instance->simpleEventInstance, iterator);
            if (!fd)
                continue;
            instance->fds[i].fd = fd->fd;
            instance->fds[i].events = 0;
            if (fd->events & SIMPLE_EVENT_READ)
                instance->fds[i].events |= POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
            if (fd->events & SIMPLE_EVENT_WRITE)
                instance->fds[i].events |= POLLOUT | POLLWRNORM | POLLWRBAND;
            simpleEventIteratorValueClean(instance->simpleEventInstance, fd);
            ++i;
        }
        simpleEventCleanIterator(instance->simpleEventInstance, iterator);
    }
    instance->modified = 0;


    int happenEventNum = poll(instance->fds, instance->maxEvents, timeout);
    if (happenEventNum < 0)
        return -1;

    int j = 0;
    for (i = 0; i < instance->maxEvents && j < happenEventNum; ++i) {
        if (!instance->fds[i].revents)
            continue;
        if (!(fd = simpleEventGetData(instance->simpleEventInstance, instance->fds[i].fd)))
            continue;
        fds[j].fd = fd->fd;
        fds[j].data = fd->data;
        if ((instance->fds[i].revents & (POLLIN | POLLRDNORM | POLLRDBAND)))
            fds[j].events &= SIMPLE_EVENT_READ;
        if ((instance->fds[i].revents & (POLLOUT | POLLWRNORM | POLLWRBAND)))
            fds[j].events &= SIMPLE_EVENT_WRITE;
        else
            fds[j].events &= SIMPLE_EVENT_EXCEPTION;
        ++j;
    }

    return j;
}

int simpleEventPollClean(simpleEventPollAdapter *instance) {
    free(instance);
    return 0;
}

simpleEventHandler createPollHandler() {
    simpleEventHandler handler;
    handler.handlerInit = &createsimpleEventPollInstance;
    handler.handlerClean = &simpleEventPollClean;
    handler.addFD = &simpleEventPollAddFD;
    handler.removeFD = &simpleEventPollRemoveFD;
    handler.handlerWait = &simpleEventPollWait;

    return handler;
}