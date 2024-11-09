#include <iostream>
#include <thread>
#include <mutex>
#include <barrier>
#include <chrono>
#include <random>
#include <semaphore>
#include <condition_variable>
#include <atomic>

using namespace std;

const int thread_count = 5; // Кол-во потоков
const int char_count = 1; // Кол-во символов, которое генеритруется потоком

mutex mtx;
counting_semaphore<1> sem(1);
mutex slim_mtx;
condition_variable slim_cv;
barrier my_barrier(thread_count);
atomic_flag spinlock = ATOMIC_FLAG_INIT;

class SemaphoreSlim {
    atomic<int> count;
    mutex mtx;
    condition_variable cv;
public:
    
    SemaphoreSlim(int initial) : count(initial) {}
    
    void acquireSlim() {
        unique_lock<mutex> lock(slim_mtx);
        while (count <= 0) {
            cv.wait(lock);
        }
        --count; // Уменьшаем счетчик
    }

    void realeseSlim() {
        unique_lock<mutex> lock(slim_mtx);
        ++count; // Уменьшаем счетчик
        cv.notify_one(); // Уведомляем один поток
    }
};

class Monitor {
    mutex m;
public:
    
    void enter() {
        m.lock();
    }

    void exit() {
        m.unlock();
    }
};

SemaphoreSlim sem_slim(1);

Monitor monitor;

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

void generateRandomCharsSemaphoreSlim(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32, 126);

    for (int i = 0; i < char_count; ++i) {
        sem_slim.acquireSlim();
        char random_char = dis(gen);
        cout << "SemaphoreSlim thread " << thread_id << ": " << random_char << endl;
        sem_slim.realeseSlim();
    }
}

void generateRandomCharsBarrier(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for (int i = 0; i < char_count; ++i) {
        char random_char = dis(gen);
        {
            lock_guard<mutex> lock(mtx);
            cout << "Barrier thread " << thread_id << ": " << random_char << endl;
        }
        my_barrier.arrive_and_wait(); // Уведомляем барьер
    }
}

void generateRandomCharsSpinLock(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for (int i = 0; i < char_count; ++i) {
        while (spinlock.test_and_set(memory_order_acquire));
        char random_char = dis(gen);
        cout << "SpinLock thread " << thread_id << ": " << random_char << endl;
        spinlock.clear(memory_order_release);
    }
}

void generateRandomCharsSpinWait(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for (int i = 0; i < char_count; ++i) {
        while (spinlock.test_and_set(memory_order_acquire)) {
            this_thread::yield();
        }
        char random_char = dis(gen);
        cout << "SpinWait thread " << thread_id << ": " << random_char << endl;
        spinlock.clear(memory_order_release);
    }
}

void generateRandomCharsMonitor(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for (int i = 0; i < char_count; ++i) {
        monitor.enter();
        char random_char = dis(gen);
        cout << "Monitor thread " << thread_id << ": " << random_char << endl;
        monitor.exit();
    }
}

int main() {
    thread threads[thread_count];

    // Потоки с использованием mutex
    for (int i = 0; i < thread_count; ++i) {
        threads[i] = thread(generateRandomCharsMutex, i);
    }

    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }

    // Потоки с использованием semaphore
    for (int i = 0; i < thread_count; ++i) {
        threads[i] =  thread(generateRandomCharsSemaphore, i);
    }

    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }

    // Потоки с использованием semaphore slim
    for (int i = 0; i < thread_count; ++i) {
        threads[i] =  thread(generateRandomCharsSemaphoreSlim, i);
    }

    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }

    // Потоки с использованием barrier
    for (int i = 0; i < thread_count; ++i) {
        threads[i] =  thread(generateRandomCharsBarrier, i);
    }

    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }

    // Потоки с использованием spinlock
    for (int i = 0; i < thread_count; ++i) {
        threads[i] =  thread(generateRandomCharsSpinLock, i);
    }

    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }

    // Потоки с использованием spinwait
    for (int i = 0; i < thread_count; ++i) {
        threads[i] =  thread(generateRandomCharsSpinWait, i);
    }

    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }

    // Потоки с использованием monitor
    for (int i = 0; i < thread_count; ++i) {
        threads[i] =  thread(generateRandomCharsMonitor, i);
    }

    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }

    return 0;
}
