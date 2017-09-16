//
// Created by zhensheng on 9/15/17.
//

#ifndef SIMPLETOOLKITS_SIMPLEEVENTEPOLL_H
#define SIMPLETOOLKITS_SIMPLEEVENTEPOLL_H

typedef struct simpleEventEpollAdapter {
    simpleEvent *simpleEventInstance;
    int epollFD;
    int maxEvents;
} simpleEventEpollAdapter;

simpleEventHandler createEpollHandler();

#endif //SIMPLETOOLKITS_SIMPLEEVENTEPOLL_H
