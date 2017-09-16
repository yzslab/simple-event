//
// Created by zhensheng on 9/16/17.
//

#include <stdlib.h>
#include "simpleEvent.h"
#include "hashTable.h"
#include "simpleEventHashTableAdapter.h"

void *simpleEventHashTableInit(unsigned int capacity) {
    return createHashTableInstance(capacity, sizeof(int), sizeof(simpleEventFD), NULL);
}

int simpleEventHashTableClean(void *instance) {
    cleanHashTableInstance(instance);
    return 0;
}

int simpleEventHashTablePut(void *instance, simpleEventFD *fd) {
    return hashTablePut(instance, &fd->fd, fd);
}

simpleEventFD *simpleEventHashTableGet(void *instance, int fd) {
    return hashTableGet(instance, &fd, NULL);
}

int simpleEventHashTableValueClean(simpleEventFD *value) {
    free(value);
    return 0;
}

int simpleEventHashTableSet(void *instance, int fd, simpleEventFD *newFD) {
    return hashTableSet(instance, &fd, newFD);
}

int simpleEventHashTableRemove(void *instance, int fd) {
    hashTableRemove(instance, &fd);
    return 0;
}

simpleEventIterator *simpleEventHashTableGetIterator(void *instance) {
    return hashTableGetIterator(instance);
}

int simpleEventHashTableIteratorHasNext(void *iterator) {
    return hashTableIteratorHasNext(iterator);
}

int simpleEventHashTableIteratorNext(void *iterator) {
    return hashTableIteratorNext(iterator);
}

simpleEventFD *simpleEventHashTableIteratorGet(void *iterator) {
    return hashTableIteratorValue(iterator);
}

int simpleEventHashTableIteratorValueClean(simpleEventFD *fd) {
    return 0;
}

int simpleEventHashTableIteratorSet(void *iterator, simpleEventFD *newFD) {
    return hashTableIteratorSet(iterator, newFD);
}

int simpleEventHashTableCleanIterator(void *iterator) {
    hashTableCleanIterator(iterator);
    return 0;
}

simpleEventContainerHandler createHashTableHandler() {
    simpleEventContainerHandler handler;

    handler.handlerInit = &simpleEventHashTableInit;
    handler.handleClean = &simpleEventHashTableClean;
    handler.put = &simpleEventHashTablePut;
    handler.get = &simpleEventHashTableGet;
    handler.clean = &simpleEventHashTableValueClean;
    handler.set = &simpleEventHashTableSet;
    handler.remove = &simpleEventHashTableRemove;
    handler.getIterator = &simpleEventHashTableGetIterator;
    handler.iteratorHasNext = &simpleEventHashTableIteratorHasNext;
    handler.iteratorNext = &simpleEventHashTableIteratorNext;
    handler.iteratorGet = &simpleEventHashTableIteratorGet;
    handler.iteratorValueClean = &simpleEventHashTableIteratorValueClean;
    handler.iteratorSet = &simpleEventHashTableIteratorSet;
    handler.cleanIterator = &simpleEventHashTableCleanIterator;

    return handler;
}