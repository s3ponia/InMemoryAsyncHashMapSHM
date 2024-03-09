# How to compile
```
cmake . -B build
cmake --build build
```
or
```
make build
```
<img width="643" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/ca7252e3-6fca-409b-9c59-bc80279f63c6">

# Task Description
* Server program:

     - Initialize a hash table of given size (command line)

     - Support insertion of items in the hash table

     - Hash table collisions are resolved by maintaining a linked list
for each bucket/entry in the hash table

     - Supports concurrent operations (multithreading) to perform
(insert, read, delete operations on the hash table)

     - Use readers-writer lock to ensure safety of concurrent
operations, try to optimize the granularity

     - Communicates with the client program using shared memory buffer
(POSIX shm)

* Client program:

     - Enqueue requests/operations (insert, read a bucket, delete) to
the server (that will operate on the the hash table) via shared memory
buffer (POSIX shm)
# Makefile
`make build`

<img width="735" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/c79cdb26-4cd3-4fb2-8a22-5f58b8005a89">

`make server BUCKETS_CNT=<bucket cnt>`

<img width="598" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/986dc458-8dc7-4452-98e1-d1534f14c53e">

`make client`

<img width="582" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/5168eaaf-fb0a-49b6-95c2-a47faa26f96a">

# Server
`./server <number of buckets in hash table>`

Prints on read request mapped value of the requested element or "no value" if there is no value with that key

<img width="635" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/babb3633-2f16-4220-9125-2dcfa55b20b5">

# Client
`./client`

`exit` request also closes server

<img width="557" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/46884511-7977-4fbd-acbc-964d98d4463f">

# Performance Test

Test contains 3 steps:
1. 1'000'000 inserts
2. 1'000'000 deletes
3. 1'000'000 reads

## Results:
with 127 buckets:
1. took 4644 ms
2. took 4926 ms
3. took 259781 ms (~260 sec)
<img width="431" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/8009cfba-1a32-435f-ac88-566fd505695d">

with 1021 buckets:
1. took 4562 ms
2. took 4867 ms
3. took 31341 ms (~30 sec)

<img width="466" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/ce6c6041-4926-4d84-85aa-b2a23174daca">


