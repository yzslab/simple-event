//
// Created by zhensheng on 9/16/17.
//

#ifndef SIMPLETOOLKITS_SIMPLEEVENTHASHTABLEADAPTER_H
#define SIMPLETOOLKITS_SIMPLEEVENTHASHTABLEADAPTER_H

void *simpleEventHashTableInit(unsigned int capacity);
int simpleEventHashTableClean(void *instance);
int simpleEventHashTablePut(void *instance, simpleEventFD *fd);
simpleEventFD *simpleEventHashTableGet(void *instance, int fd);
int simpleEventHashTableValueClean(simpleEventFD *value);
int simpleEventHashTableSet(void *instance, int fd, simpleEventFD *newFD);
int simpleEventHashTableRemove(void *instance, int fd);
simpleEventIterator *simpleEventHashTableGetIterator(void *instance);
int simpleEventHashTableIteratorHasNext(void *iterator);
int simpleEventHashTableIteratorNext(void *iterator);
simpleEventFD *simpleEventHashTableIteratorGet(void *iterator);
int simpleEventHashTableIteratorValueClean(simpleEventFD *fd);
int simpleEventHashTableIteratorSet(void *iterator, simpleEventFD *newFD);
int simpleEventHashTableCleanIterator(void *iterator);
simpleEventContainerHandler createHashTableHandler();

#endif //SIMPLETOOLKITS_SIMPLEEVENTHASHTABLEADAPTER_H
