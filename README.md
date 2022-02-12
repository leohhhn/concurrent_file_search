# concurrent_file_search

This program searches given dirs for .txt files, and counts the number of prime numbers inside of them.
To do this, the program utilises a C concurrency library, pthreads. 
In order to achieve concurrency, it uses the producer/consumer style of synchronisation between threads that watch dirs (watchers), and threads that scan .txt files (workers). 