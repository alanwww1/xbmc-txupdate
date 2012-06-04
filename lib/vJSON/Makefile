CC = g++

libvjson.a : block_allocator.o json.o
	ar r -o libvjson.a block_allocator.o json.o

block_allocator.o : block_allocator.cpp block_allocator.h
	$(CC) -g -c block_allocator.cpp

json.o : json.h json.cpp
	$(CC) -g -c json.cpp

