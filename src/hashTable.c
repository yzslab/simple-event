//
// Created by zhensheng on 9/16/17.
//

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "hashTable.h"

#define hashTableGetPosition(instance, hashValue) ((hashValue) % (instance)->capacity)
#define hashTableGetRealPosition(instance, hashValue) (hashTableGetPosition(instance, hashValue) * (instance)->perBucketSize)

// Private functions
static hashTableHashValue defaultHashFunction(void *key, size_t bytes);
static hashTableBucket *hashTableFindKey(hashTable *instance, void *key);
static hashTablePosition hashTableIteratorFindNext(hashTableIterator *iterator);
static int hashTableSynchronizeStart(hashTable *instance);
static int hashTableSynchronizeEnd(hashTable *instance);

hashTable *createHashTableInstance(unsigned int capacity, size_t keySize, size_t valueSize, hashTableHashFunction hashFunction) {
    if (!(capacity & 1))
        ++capacity;

    size_t perBucketSize = sizeof(hashTableBucket) + keySize + valueSize;
    size_t totalBucketSize = capacity * perBucketSize;
    size_t structSize = sizeof(hashTable) + totalBucketSize;

    hashTable *instance = malloc(structSize);

    if (!instance)
        return NULL;

    if (pthread_mutexattr_init(&instance->attr) != 0)
        goto FREE_ON_ERROR;
    if (pthread_mutexattr_settype(&instance->attr, PTHREAD_MUTEX_RECURSIVE) != 0)
        goto DESTROY_MUTEX_ATTR_ON_ERROR;
    if (pthread_mutex_init(&instance->mutex, &instance->attr) != 0)
        goto FREE_ON_ERROR;
    instance->capacity = capacity;
    instance->filledNum = 0;
    instance->keySize = keySize;
    instance->valueSize = valueSize;
    instance->error = HASH_TABLE_ERROR_NONE;
    instance->perBucketSize = perBucketSize;
    instance->totalBucketSize = totalBucketSize;
    instance->hashFunction = hashFunction ? hashFunction : &defaultHashFunction;

    memset(instance->buckets, 0, totalBucketSize);

    return instance;

    DESTROY_MUTEX_ON_ERROR:
    pthread_mutex_destroy(&instance->mutex);
    DESTROY_MUTEX_ATTR_ON_ERROR:
    pthread_mutexattr_destroy(&instance->attr);
    FREE_ON_ERROR:
    free(instance);
    ON_ERROR:
    return NULL;
}

int hashTablePut(hashTable *instance, void *key, void *value) {
    if (hashTableSynchronizeStart(instance))
        return -1;
    int returnValue = 0;
    if (instance->capacity == instance->filledNum) {
        instance->error = HASH_TABLE_ERROR_NO_FREE_BUCKET;
        returnValue = -1;
        goto END;
    }

    ++instance->filledNum;

    hashTablePosition realPosition = hashTableGetRealPosition(instance, instance->hashFunction(key, instance->keySize));

    for (; instance->buckets[realPosition].filled;) {
        realPosition += instance->perBucketSize;
        if (realPosition == instance->capacity)
            realPosition = 0;
    }

    instance->buckets[realPosition].filled = 1;
    memcpy(&instance->buckets[realPosition].key_value, key, instance->keySize);
    memcpy(&instance->buckets[realPosition].key_value[instance->keySize], value, instance->valueSize);

    END:
    hashTableSynchronizeEnd(instance);
    return returnValue;
}

void *hashTableGet(hashTable *instance, void *key, void *valueBuffer) {
    if (hashTableSynchronizeStart(instance))
        return NULL;

    hashTableBucket *bucket;
    if (!(bucket = hashTableFindKey(instance, key))) {
        instance->error = HASH_TABLE_ERROR_KEY_NOT_EXISTS;
        valueBuffer = NULL;
        goto END;
    }

    if (!valueBuffer)
        valueBuffer = malloc(instance->valueSize);

    if (!valueBuffer) {
        instance->error = HASH_TABLE_ERROR_SYSTEM_CALL;
        valueBuffer = NULL;
        goto END;
    }

    memcpy(valueBuffer, &bucket->key_value[instance->keySize], instance->valueSize);

    END:
    hashTableSynchronizeEnd(instance);
    return valueBuffer;
}

int hashTableSet(hashTable *instance, void *key, void *value) {
    if (hashTableSynchronizeStart(instance))
        return -1;

    int returnValue = 0;

    hashTableBucket *bucket;
    if (!(bucket = hashTableFindKey(instance, key))) {
        instance->error = HASH_TABLE_ERROR_KEY_NOT_EXISTS;
        returnValue = -1;
        goto END;
    }

    memcpy(&bucket->key_value[instance->keySize], value, instance->valueSize);

    END:
    hashTableSynchronizeEnd(instance);
    return returnValue;
}

int hashTableRemove(hashTable *instance, void *key) {
    if (hashTableSynchronizeStart(instance))
        return -1;

    hashTableBucket *bucket;
    if (!(bucket = hashTableFindKey(instance, key)))
        goto END;

    bucket->filled = 0;
    --instance->filledNum;

    END:
    hashTableSynchronizeEnd(instance);
    return 0;
}

void cleanHashTableInstance(hashTable *instance) {
    pthread_mutexattr_destroy(&instance->attr);
    pthread_mutex_destroy(&instance->mutex);
    free(instance);
}

hashTableIterator *hashTableGetIterator(hashTable *instance) {
    hashTableIterator *iterator = malloc(sizeof(hashTableIterator) + instance->keySize + instance->valueSize);
    if (!iterator)
        return NULL;

    iterator->instance = instance;
    iterator->position = -instance->perBucketSize;
    iterator->nextPosition = iterator->position;

    return iterator;
}

int hashTableIteratorHasNext(hashTableIterator *iterator) {
    iterator->nextPosition = hashTableIteratorFindNext(iterator);
    return iterator->position < iterator->nextPosition;
}

static hashTablePosition hashTableIteratorFindNext(hashTableIterator *iterator) {
    if (iterator->nextPosition > iterator->position)
        return iterator->nextPosition;

    hashTablePosition position = iterator->position + iterator->instance->perBucketSize;

    for (; position < iterator->instance->totalBucketSize; position += iterator->instance->perBucketSize)
        if (iterator->instance->buckets[position].filled)
            return position;

    return iterator->position;
}

int hashTableIteratorNext(hashTableIterator *iterator) {
    iterator->position = iterator->nextPosition;
    memcpy(iterator->key_value, iterator->instance->buckets[iterator->position].key_value, iterator->instance->keySize + iterator->instance->valueSize);
    return 0;
}

void *hashTableIteratorKey(hashTableIterator *iterator) {
    if (hashTableSynchronizeStart(iterator->instance))
        return NULL;
    memcpy(iterator->key_value, iterator->instance->buckets[iterator->position].key_value, iterator->instance->keySize);
    hashTableSynchronizeEnd(iterator->instance);
    return iterator->key_value;
}

void *hashTableIteratorValue(hashTableIterator *iterator) {
    if (hashTableSynchronizeStart(iterator->instance))
        return NULL;
    memcpy(&iterator->key_value[iterator->instance->keySize], iterator->instance->buckets[iterator->position].key_value, iterator->instance->keySize);
    hashTableSynchronizeEnd(iterator->instance);
    return &iterator->key_value[iterator->instance->keySize];
}

int hashTableIteratorSet(hashTableIterator *iterator, void *value) {
    int returnValue;
    if ((returnValue = hashTableSet(iterator->instance, hashTableIteratorKey(iterator), value)))
        return returnValue;
    return 0;
}

int hashTableIteratorRemove(hashTableIterator *iterator) {
    hashTableRemove(iterator->instance, hashTableIteratorKey(iterator));
    return 0;
}

void hashTableCleanIterator(hashTableIterator *iterator) {
    free(iterator);
}

hashTableBucket *hashTableFindKey(hashTable *instance, void *key) {
    if (hashTableSynchronizeStart(instance))
        return NULL;
    hashTablePosition startPosition = hashTableGetRealPosition(instance, instance->hashFunction(key, instance->keySize));
    hashTablePosition position = startPosition;
    do {
        if (instance->buckets[position].filled && !memcmp(&instance->buckets[position].key_value, key, instance->keySize)) {
            hashTableSynchronizeEnd(instance);
            return &instance->buckets[position];
        }
        position += instance->perBucketSize;
        if (position >= instance->totalBucketSize)
            position = 0;
    } while (position != startPosition);

    hashTableSynchronizeEnd(instance);
    return NULL;
}

hashTableHashValue defaultHashFunction(void *key, size_t bytes) {
    hashTableHashValue hashValue = 0;
    int i, j;

    for (i = 0, j = 0; i < bytes; ++i, ++j, ++key) {
        if (j == 4)
            j = 0;
        hashValue += (*(uint8_t *) key) << (j * 8);
    }

    return hashValue;
}

int hashTableSynchronizeStart(hashTable *instance) {
    return pthread_mutex_lock(&instance->mutex);
}

int hashTableSynchronizeEnd(hashTable *instance) {
    return pthread_mutex_unlock(&instance->mutex);
}