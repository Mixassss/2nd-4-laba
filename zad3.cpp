#include "RandW.h"

WriterAndReader::WriterAndReader(bool check) : writers(0), readers(0), priority(check) {}

void WriterAndReader::startWriting() {
    unique_lock<mutex> lock(mx);
    if (priority) {
        while (writers > 0) {
            cv.wait(lock); // Ждём других писателей
        }
        writers++;
    } else {
        while (readers > 0 || writers > 0) {
            cv.wait(lock); // Ждём читателей и писателей
        }
        writers++;
    }
}

void WriterAndReader::stopWriting() {
    unique_lock<mutex> lock(mx);
    writers--;
    if (writers == 0) {
        cv.notify_one(); // Уведомляем читателей
    }
}

void WriterAndReader::startReading() {
    unique_lock<mutex> lock(mx);
    if (!priority) {
        readers++;
    } else {
        while (writers > 0) {
            cv.wait(lock); // Ждем всех писателей
        }
        readers++;
    }
}

void WriterAndReader::stopReading() {
    unique_lock<mutex> lock(mx);
    readers--;
    cv.notify_all(); // Уведомляем писателей
}

void WriterAndReader::replacePriority(bool change) {
    priority = change;
}

void priority_writer(WriterAndReader& raw, int id, int iterations) { // Приоритет писателя
    for (int i = 0; i < iterations; ++i) {
        raw.startWriting();
        cout << "Писатель " << id << " пишет" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
        raw.stopWriting();
    }
}

void priority_reader(WriterAndReader& raw, int id, int iterations) { // Приоритет читателя
    for (int i = 0; i < iterations; ++i) {
        raw.startReading();
        cout << "Читатель " << id << " занят читением" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
        raw.stopReading();
    }
}

int main() {

    WriterAndReader raw(true);
    int iterations = 1;
    int write_limit = 2; // Количество записей для каждого писателя
    int read_limit = 2; // Количество чтений для каждого читателя

    cout << endl << "Выбран приоритет писателя: " << endl;
    vector<thread> threadsWriter(write_limit);
    vector<thread> threadsReader(read_limit);
    for (int i = 0; i < write_limit; ++i) {
        threadsWriter[i] = thread(priority_writer, ref(raw), i+1, iterations);
    }

    for (int i = 0; i < read_limit; ++i) {
        threadsReader[i] = thread(priority_reader, ref(raw), i+1, iterations);
    }
    for (auto& t : threadsWriter) {
        t.join();
    }
    for (auto& t : threadsReader) {
        t.join();
    }

    raw.replacePriority(false);

    cout << endl << "Выбран приоритет читателя: " << endl;
    threadsWriter.clear();
    threadsReader.clear();
    for (int i = 0; i < write_limit; ++i) {
        threadsWriter[i] = thread(priority_writer, ref(raw), i + 1, iterations);
    }
    for (int i = 0; i < read_limit; ++i) {
        threadsReader[i] = thread(priority_reader, ref(raw), i + 1, iterations);
    }
    for (auto& t : threadsWriter) {
        t.join();
    }
    for (auto& t : threadsReader) {
        t.join();
    }


    return 0;
}
