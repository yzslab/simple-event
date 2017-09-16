//
// Created by zhensheng on 9/17/17.
//

#ifndef SIMPLETOOLKITS_SIMPLEEVENTPOOLADAPTER_H
#define SIMPLETOOLKITS_SIMPLEEVENTPOOLADAPTER_H

#include <poll.h>

typedef struct simpleEventPollAdapter {
    simpleEvent *simpleEventInstance;
    int maxEvents;
    int modified;
    struct pollfd fds[];
} simpleEventPollAdapter;

simpleEventHandler createPollHandler();

#endif //SIMPLETOOLKITS_SIMPLEEVENTPOOLADAPTER_H
