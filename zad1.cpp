#include "utility.h"

void generateRandomCharsMutex(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for (int i = 0; i < char_count; ++i) {
        char random_char = dis(gen);
        lock_guard<mutex> lock(mtx);
        cout << "Thread " << thread_id << ": " << random_char << endl;
    }
}

void generateRandomCharsSemaphore(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы

    for(int i = 0; i < char_count; ++i) {
        char random_char = dis(gen);
        sem_wait(&sem);
        cout << "Thread " << thread_id << ": " << random_char << endl;
    }
}

void generateRandomCharsBarrier(int thread_id) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(32,126); // Генерация случайных символов из ASII таблицы


}
