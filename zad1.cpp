#include <iostream>
#include <thread>
#include <mutex>
#include <barrier>
#include <chrono>
#include <random>
#include <semaphore>

using namespace std;

const int thread_count = 10; // Кол-во потоков
const int char_count = 5; // Кол-во символов, которое генеритруется потоком

mutex mtx;
counting_semaphore<1> sem(1);
barrier<> my_barrier(thread_count);

void generateRandomCharsMutex(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for (int i = 0; i < char_count; ++i) {
        char random_char = dis(gen);
        lock_guard<mutex> lock(mtx);
        cout << "Mutex thread " << thread_id << ": " << random_char << endl;
    }
}

void generateRandomCharsSemaphore(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32, 126);

    for(int i = 0; i < char_count; ++i) {
        sem.acquire();
        char random_char = dis(gen);
        cout << "Semaphore thread " << thread_id << ": " << random_char << endl;
        sem.release();
    }
}

void generateRandomCharsBarrier(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for (int i = 0; i < char_count; ++i) {
        char random_char = dis(gen);
        cout << "Barrier thread " << thread_id << ": " << random_char << endl;
        my_barrier.arrive_and_wait(); // Уведомляем барьер
    }
}

int main() {
    // Запуск потоков с mutex
    for (int i = 0; i < thread_count; ++i) {
        thread t(generateRandomCharsMutex, i);
        t.join();
    }

    // Запуск потоков с semaphore
    for (int i = 0; i < thread_count; ++i) {
        thread t(generateRandomCharsSemaphore, i);
        t.join();
    }

    // Запуск потоков с barrier
    for (int i = 0; i < thread_count; ++i) {
        thread t(generateRandomCharsBarrier, i);
        t.join();
    }

    return 0;
}
