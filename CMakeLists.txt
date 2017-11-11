cmake_minimum_required(VERSION 3.8)
project(simpleToolkits)

set(CMAKE_C_STANDARD 99)

add_library(simpleTCPServer SHARED src/simpleTCPServer.c src/simpleTCPServer.h)
add_library(simpleEvent SHARED src/simpleEvent.c src/simpleEvent.h src/simpleEventEpollAdapter.c src/simpleEventEpollAdapter.h src/simpleEventHashTableAdapter.c src/simpleEventHashTableAdapter.h src/simpleEventSelectAdapter.c src/simpleEventSelectAdapter.h src/simpleEventPollAdapter.c src/simpleEventPollAdapter.h)
add_library(hashTable SHARED src/hashTable.c src/hashTable.h)
add_library(simpleTCPServerStatic STATIC src/simpleTCPServer.c src/simpleTCPServer.h)
add_library(simpleEventStatic STATIC src/simpleEvent.c src/simpleEvent.h src/simpleEventEpollAdapter.c src/simpleEventEpollAdapter.h src/simpleEventHashTableAdapter.c src/simpleEventHashTableAdapter.h src/simpleEventSelectAdapter.c src/simpleEventSelectAdapter.h src/simpleEventPollAdapter.c src/simpleEventPollAdapter.h)
add_library(hashTableStatic STATIC src/hashTable.c src/hashTable.h)
target_link_libraries(hashTable pthread)
target_link_libraries(hashTableStatic pthread)

add_executable(Test main.c)
target_link_libraries(Test simpleTCPServer)
target_link_libraries(Test simpleEvent)
target_link_libraries(Test hashTable)