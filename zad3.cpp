#include "RandW.h"

WriterAndReader::WriterAndReader() : nreaders(0) {}

void WriterAndReader::read(int id, int read_limit) {
    for (int i = 0; i < read_limit; ++i) {
        read_priority();
        
        cout << "Читатель " << id << " занят чтением" << endl;
        this_thread::sleep_for(chrono::milliseconds(100));

        // Завершение чтения
        {
            lock_guard<mutex> lock(mut_counter);
            nreaders--;
            if (nreaders == 0) {
                rw_mutex.unlock(); // Последний читатель освобождает доступ писателям
                cout << "Последний читатель " << id << " завершил чтение и освободил доступ писателям." << endl;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void WriterAndReader::write(int id, int write_limit) {
    for (int i = 0; i < write_limit; ++i) {
        write_priority();
        
        cout << "Писатель " << id << " получил доступ и пишет." << endl;
        this_thread::sleep_for(chrono::milliseconds(100));
        
        // Освобождаем блокировку
        rw_mutex.unlock();
        cout << "Писатель " << id << " завершил запись и освободил доступ." << endl;
        
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void WriterAndReader::read_priority() {
    lock_guard<mutex> lock(mut_counter);
    nreaders++;
    if (nreaders == 1) {
        rw_mutex.lock(); // Первый читатель блокирует доступ писателям
        cout << "Первый читатель начал чтение и заблокировал доступ для писателей." << endl;
    }
}

void WriterAndReader::write_priority() {
    rw_mutex.lock(); // Блокируем доступ всем читателям и писателям
    cout << "Писатель заблокировал доступ для всех читателей и писателей." << endl;
}

int main() {
    WriterAndReader raw;

    thread readers[2];
    thread writers[2];

    int read_limit = 3;  // Количество чтений для каждого читателя
    int write_limit = 2; // Количество записей для каждого писателя

    for (int i = 0; i < 2; i++) { // Создание потоков читателей
        readers[i] = thread(&WriterAndReader::read, &raw, i + 1, read_limit);
    }

    for (int i = 0; i < 2; i++) { // Создание потоков писателей
        writers[i] = thread(&WriterAndReader::write, &raw, i + 1, write_limit);
    }

    for (int i = 0; i < 2; i++) { // Ждать завершения потоков
        readers[i].join();
    }

    for (int i = 0; i < 2; i++) { // Ждать завершения потоков писателей
        writers[i].join();
    }

    return 0;
}
