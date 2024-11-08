#ifndef utility_h
#define utility_h

#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore.h>
#include <barrier>
#include <chrono>
#include <random>

using namespace std;

const int thread_count = 10; // Кол-во потоков
const int char_count = 10; // Кол-во символов, которое генеритруется потоком

mutex mtx;
sem_t sem;

#endif // UTILITY_H