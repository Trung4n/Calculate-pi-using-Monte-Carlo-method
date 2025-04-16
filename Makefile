CC = gcc
CFLAGS = -Wall -Wextra -O3 -lpthread
TARGET = approach

default:
	$(CC) $(CFLAGS) -DMUTEX -o $(TARGET) approach.c
atomic:
	$(CC) $(CFLAGS) -DREDUCE_ATOMIC -o $(TARGET) approach.c
lockfree:
	$(CC) $(CFLAGS) -DREDUCE_LOCK_FREE -o $(TARGET) approach.c
threadlocal:
	$(CC) $(CFLAGS) -DMUTEX -DREDUCE_THREAD_LOCAL -o $(TARGET) approach.c
