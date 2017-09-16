//
// Created by zhensheng on 9/16/17.
//

#ifndef SIMPLETOOLKITS_HASHTABLE_H
#define SIMPLETOOLKITS_HASHTABLE_H

#include <stdint.h>
#include <pthread.h>

typedef unsigned long hashTableHashValue;
typedef long hashTablePosition;
typedef hashTableHashValue (*hashTableHashFunction)(void *, size_t);

typedef enum hashTableError {
    HASH_TABLE_ERROR_NONE,
    HASH_TABLE_ERROR_SYSTEM_CALL,
    HASH_TABLE_ERROR_KEY_NOT_EXISTS,
    HASH_TABLE_ERROR_NO_FREE_BUCKET,
} hashTableError;

typedef struct hashTableBucket {
    uint8_t filled; // If filled is non-zero, bucket has been filled.
    uint8_t key_value[]; // First hashTable->keySize bytes is key, following hashTable->valueSize bytes value.
} hashTableBucket;

typedef struct hashTable {
    pthread_mutexattr_t attr;
    pthread_mutex_t mutex;
    hashTableHashFunction hashFunction; // Hash value calculator
    unsigned int capacity; // How many buckets should i have?
    unsigned int filledNum; // How many buckets have been filled.
    size_t keySize;
    size_t valueSize;
    hashTableError error;
    size_t perBucketSize;
    size_t totalBucketSize;
    hashTableBucket buckets[];
} hashTable;

typedef struct hashTableIterator {
    struct hashTable *instance;
    hashTablePosition position;
    hashTablePosition nextPosition;
    uint8_t key_value[];
} hashTableIterator;


// Public functions
hashTable *createHashTableInstance(unsigned int capacity, size_t keySize, size_t valueSize, hashTableHashFunction hashFunction);
int hashTablePut(hashTable *instance, void *key, void *value);
void *hashTableGet(hashTable *instance, void *key, void *valueBuffer);
int hashTableSet(hashTable *instance, void *key, void *value);
int hashTableRemove(hashTable *instance, void *key);
void cleanHashTableInstance(hashTable *instance);
hashTableIterator *hashTableGetIterator(hashTable *instance);
int hashTableIteratorHasNext(hashTableIterator *iterator);
int hashTableIteratorNext(hashTableIterator *iterator);
void *hashTableIteratorKey(hashTableIterator *iterator);
void *hashTableIteratorValue(hashTableIterator *iterator);
int hashTableIteratorSet(hashTableIterator *iterator, void *value);
int hashTableIteratorRemove(hashTableIterator *iterator);
void hashTableCleanIterator(hashTableIterator *iterator);


#endif //SIMPLETOOLKITS_HASHTABLE_H
