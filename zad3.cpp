#include "RandW.h"

WriterAndReader::WriterAndReader() : nreaders(0), writer_active(false) {}

void WriterAndReader::read(int id, int read_limit) {
    for (int i = 0; i < read_limit; ++i) {
        {
            lock_guard<mutex> lock(mut_counter);
            if (nreaders == 0) {
                no_writes.lock(); // Первый читатель блокирует доступ писателям
            }
            nreaders++;
        }

        cout << "Читатель " << id << " занят чтением" << endl;
        this_thread::sleep_for(chrono::milliseconds(100));

        {
            lock_guard<mutex> lock(mut_counter);
            nreaders--;
            if (nreaders == 0) {
                no_writes.unlock(); // Последний читатель освобождает доступ писателям
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void WriterAndReader::write(int id, int write_limit) {
    for (int i = 0; i < write_limit; ++i) {
        no_writes.lock();  // Блокируем доступ для других писателей и читателей
        writer_active = true; // Устанавливаем флаг активности писателя

        cout << "Писатель " << id << " пишет" << endl;
        this_thread::sleep_for(chrono::milliseconds(100));

        writer_active = false; // Сбрасываем флаг активности писателя
        no_writes.unlock(); // Освобождаем блокировку для других писателей и читателей            
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void WriterAndReader::set_priority(bool readerPriority) {
    // Установка приоритета
    if (readerPriority) {
        cout << "Приоритет читателя установлен." << endl;
    } else {
        cout << "Приоритет писателя установлен." << endl;
    }
}

int main() {
    WriterAndReader raw;

    thread readers[3];
    thread writers[3];
    
    int read_limit = 3;  // Количество чтений для каждого читателя
    int write_limit = 3; // Количество записей для каждого писателя

    // Запрос приоритета у пользователя
    char choice;
    cout << "Введите 'r' для приоритета читателей или 'w' для приоритета писателей: ";
    cin >> choice;
    
    if (choice == 'r') {
        raw.set_priority(true); // Приоритет читателя
    } else {
        raw.set_priority(false); // Приоритет писателя
    }

    for (int i = 0; i < 3; i++) { // Создание потоков читателей
        readers[i] = thread(&WriterAndReader::read, &raw, i + 1, read_limit);
    }

    for (int i = 0; i < 3; i++) { // Создание потоков писателей
        writers[i] = thread(&WriterAndReader::write, &raw, i + 1, write_limit);
    }

    for (int i = 0; i < 3; i++) { // Ждать завершения потоков
        readers[i].join();
    }
    for (int i = 0; i < 3; i++) {
        writers[i].join();
    }

    return 0;
}
