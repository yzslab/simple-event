//
// Created by zhensheng on 9/15/17.
//

#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "simpleEvent.h"
#include "simpleEventEpollAdapter.h"

static simpleEventEpollAdapter *createSimpleEventEpollInstance(simpleEvent *simpleEventInstance, int maxEvents);
static int simpleEventEpollAddFD(simpleEventEpollAdapter *, simpleEventFD *);
static int simpleEventEpollRemoveFD(simpleEventEpollAdapter *, int);
static int simpleEventEpollWait(simpleEventEpollAdapter *adapterInstance, simpleEventFD *fds, int timeout);
static int simpleEventEpollClean(simpleEventEpollAdapter *);

simpleEventEpollAdapter *createSimpleEventEpollInstance(simpleEvent *simpleEventInstance, int maxEvents) {
    simpleEventEpollAdapter *instance = malloc(sizeof(simpleEventEpollAdapter));
    if (!instance)
        goto ON_ERROR;

    instance->simpleEventInstance = simpleEventInstance;
    instance->epollFD = epoll_create(maxEvents);
    if (instance->epollFD < 0)
        goto FREE_ON_ERROR;

    instance->maxEvents = maxEvents;

    return instance;

    FREE_ON_ERROR:
    free(instance);
    ON_ERROR:
    return NULL;
}

int simpleEventEpollAddFD(simpleEventEpollAdapter *instance, simpleEventFD *fd) {
    struct epoll_event ev;
    ev.events = (fd->events & SIMPLE_EVENT_READ ? EPOLLIN | EPOLLPRI : 0) | (fd->events & SIMPLE_EVENT_WRITE ? EPOLLOUT : 0) | (fd->events & SIMPLE_EVENT_EXCEPTION ? EPOLLRDHUP : 0);
    ev.data.fd = fd->fd;
    return epoll_ctl(instance->epollFD, EPOLL_CTL_ADD, fd->fd, &ev);
}

int simpleEventEpollRemoveFD(simpleEventEpollAdapter *instance, int fd) {
    return epoll_ctl(instance->epollFD, EPOLL_CTL_DEL, fd, NULL);
}

int simpleEventEpollWait(simpleEventEpollAdapter *adapterInstance, simpleEventFD *fds, int timeout) {
    struct epoll_event evlist[adapterInstance->maxEvents];

    int happenEventNum;
    happenEventNum = epoll_wait(adapterInstance->epollFD, evlist, adapterInstance->maxEvents, timeout);
    if (happenEventNum < 0)
        return -1;
    else if (!happenEventNum)
        return 0;

    simpleEventFD *fdPtr;
    int i, realHappenEventNum, events;
    for (i = 0, realHappenEventNum = 0; i < happenEventNum; ++i) {
        if (!(fdPtr = simpleEventGetData(adapterInstance->simpleEventInstance, evlist[i].data.fd)))
            continue;
        ++realHappenEventNum;
        fds[i] = *fdPtr;
        events = (evlist[i].events & EPOLLIN ? SIMPLE_EVENT_READ : 0) | (evlist[i].events & EPOLLPRI ? SIMPLE_EVENT_READ : 0) | (evlist[i].events & EPOLLRDHUP ? SIMPLE_EVENT_EXCEPTION : 0) | (evlist[i].events & EPOLLOUT ? SIMPLE_EVENT_WRITE : 0) | (evlist[i].events & EPOLLERR ? SIMPLE_EVENT_EXCEPTION : 0) | (evlist[i].events & EPOLLHUP ? SIMPLE_EVENT_EXCEPTION : 0);
        fds[i].events &= events;
    }

    return realHappenEventNum;
}

int simpleEventEpollClean(simpleEventEpollAdapter *instance) {
    close(instance->epollFD);
    free(instance);
    return 0;
}

simpleEventHandler createEpollHandler() {
    simpleEventHandler handler;
    handler.handlerInit = &createSimpleEventEpollInstance;
    handler.handlerClean = &simpleEventEpollClean;
    handler.addFD = &simpleEventEpollAddFD;
    handler.removeFD = &simpleEventEpollRemoveFD;
    handler.handlerWait = &simpleEventEpollWait;

    return handler;
}