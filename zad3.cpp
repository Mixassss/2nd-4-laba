#include "R&W.h"

WriterAndReader::WriterAndReader() : nreaders(0) {}

void WriterAndReader::read(int id, int read_limit) {
    for (int i = 0; i < read_limit; ++i) {
        no_writes.lock(); // Блокируем доступ писателям
            
        { // Начало чтения
            lock_guard<mutex> lock(mut_counter);
            nreaders++;
            if (nreaders == 1) {
                no_reads.lock(); // Первый читатель блокирует доступ писателям
            }
        }

        no_writes.unlock(); // Освобождаем блокировку для писателей

        cout << "Читатель " << id << " занят чтением" << endl;
        this_thread::sleep_for(chrono::milliseconds(100));

        { // Завершение чтения
            lock_guard<mutex> lock(mut_counter);
            nreaders--;
            if (nreaders == 0) {
                no_reads.unlock(); // Последний читатель освобождает доступ писателям
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void WriterAndReader::write(int id, int write_limit) {
    for (int i = 0; i < write_limit; ++i) {
        no_writes.lock(); // Блокируем других писателей
        no_reads.lock();  // Блокируем доступ читателям

        cout << "Писатель " << id << " пишет" << endl;
        this_thread::sleep_for(chrono::milliseconds(100));

        no_reads.unlock(); // Освобождаем блокировку для читателей
        no_writes.unlock(); // Освобождаем блокировку для других писателей
            
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

int main() {
    WriterAndReader raw;

    thread readers[5];
    thread writers[2];
    
    int read_limit = 5;  // Количество чтений для каждого читателя
    int write_limit = 3; // Количество записей для каждого писателя

    for (int i = 0; i < 5; i++) { // Создание потоков читателей
        readers[i] = thread(&WriterAndReader::read, &raw, i + 1, read_limit);
    }

    for (int i = 0; i < 2; i++) { // Создание потоков писателей
        writers[i] = thread(&WriterAndReader::write, &raw, i + 1, write_limit);
    }

    for (int i = 0; i < 5; i++) { // Ждать завершения потоков
        readers[i].join();
    }
    for (int i = 0; i < 2; i++) {
        writers[i].join();
    }

    return 0;
}