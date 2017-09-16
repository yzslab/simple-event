# Simple Event Library
I/O multiplexing abstract layer.

## Core
Define the interfaces of I/O multiplexing and container
```
src/simpleEvent.h
src/simpleEvent.c
```

## Implemented I/O multiplexing modal
### select()
```
src/simpleEventSelectAdapter.h
src/simpleEventSelectAdapter.c
```
### poll()
```
src/simpleEventPollAdapter.h
src/simpleEventPollAdapter.c
```
### epoll()
```
src/simpleEventEpollAdapter.h
src/simpleEventEpollAdapter.c
```

## Some extra modules
### Generic hash table
```
src/hashTable.c
src/hashTable.h

src/simpleEventHashTableAdapter.h
src/simpleEventHashTableAdapter.c
```
### Simple TCP server
```
src/simpleTCPServer.h
src/simpleTCPServer.c
```