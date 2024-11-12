#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

class WriterAndReader {
private:
    mutex no_reads;
    mutex no_writes;
    mutex mut_counter;
    int nreaders = 0;

public:
    void read(int id) {
        while (true) {
            no_writes.lock(); // Блокируем доступ писателям
            {
                lock_guard<mutex> lock(mut_counter);
                nreaders++;
                if (nreaders == 1) {
                    no_reads.lock(); // Первый читатель блокирует доступ писателям
                }
            }
            no_writes.unlock(); // Освобождаем блокировку для писателей

            cout << "Читатель " << id << " занят чтением" << endl;
            this_thread::sleep_for(chrono::milliseconds(100));

            {
                lock_guard<mutex> lock(mut_counter); // Завершаем чтение
                nreaders--;
                if (nreaders == 0) {
                    no_reads.unlock(); // Последний читатель освобождает писателям
                }
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }

    void write(int id) {
        while (true) {
            no_writes.lock(); // Блокируем других писателей
            no_reads.lock(); // Блокируем доступ читателям

            cout << "Писатель " << id << " пишет" << endl;
            this_thread::sleep_for(chrono::milliseconds(100));

            no_reads.unlock();
            no_writes.unlock();
        }
    }
};

int main() {
    WriterAndReader raw;

    thread readers[5];
    thread writers[2];

    // Создание потоков читателей
    for (int i = 0; i < 5; i++) {
        readers[i] = thread(&WriterAndReader::read, &raw, i + 1);
    }

    // Создание потоков писателей
    for (int i = 0; i < 2; i++) {
        writers[i] = thread(&WriterAndReader::write, &raw, i + 1);
    }

    // Ждать завершения потоков
    for (int i = 0; i < 5; i++) {
        readers[i].join();
    }
    for (int i = 0; i < 2; i++) {
        writers[i].join();
    }

    return 0;
}