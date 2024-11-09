#include <iostream>
#include <thread>
#include <mutex>
#include <barrier>
#include <chrono>
#include <random>
#include <semaphore>
#include <condition_variable>
#include <atomic>
#include <vector>

using namespace std;

const int thread_count = 10; // Кол-во потоков
const int char_count = 5; // Кол-во символов, которое генеритруется потоком

mutex mtx;
counting_semaphore<1> sem(1);
mutex slim_mtx;
condition_variable slim_cv;
int slim_count = 0;
const int slim_limit = 1; // Ограничение семафора slim
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

void acquireSlim() {
    unique_lock<mutex> lock(slim_mtx);
    while (slim_count >= slim_limit) {
        slim_cv.wait(lock);
    }
    ++slim_count; // Увеличиваем счетчик
}

void realeseSlim() {
    unique_lock<mutex> lock(slim_mtx);
    --slim_count; // Уменьшаем счетчик
    slim_cv.notify_one(); // Уведомляем один поток
}

void generateRandomCharsSemaphoreSlim(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32, 126);

    for (int i = 0; i < char_count; ++i) {
        acquireSlim();
        char random_char = dis(gen);
        cout << "SemaphoreSlim thread " << thread_id << ": " << random_char << endl;
        realeseSlim();
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

void generateRandomCharsSpinLock(int thread_id) {

}

int main() {
    vector<thread> threads;

    // Потоки с использованием mutex
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(generateRandomCharsMutex, i);
    }

    for (auto& t : threads) {
        t.join();
    }
    threads.clear();

    // Потоки с использованием semaphore
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(generateRandomCharsSemaphore, i);
    }

    for (auto& t : threads) {
        t.join();
    }
    threads.clear();

    // Потоки с использованием semaphore slim
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(generateRandomCharsSemaphoreSlim, i);
    }

    for (auto& t : threads) {
        t.join();
    }
    threads.clear();

    // Потоки с использованием barrier
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(generateRandomCharsBarrier, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
