//
// Created by zhensheng on 9/17/17.
//

#ifndef SIMPLETOOLKITS_SIMPLEEVENTSELECTADAPTER_H
#define SIMPLETOOLKITS_SIMPLEEVENTSELECTADAPTER_H

#include <sys/select.h>
#include "simpleEvent.h"

typedef struct simpleEventSelectAdapter simpleEventSelectAdapter;

struct simpleEventSelectAdapter {
    simpleEvent *simpleEventInstance;
    int maxEvents;
    fd_set readFDs;
    fd_set writeFDs;
    fd_set exceptFDs;
};

simpleEventHandler createSelectHandler();

#endif //SIMPLETOOLKITS_SIMPLEEVENTSELECTADAPTER_H
