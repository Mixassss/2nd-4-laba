#ifndef RandW_h
#define RandW_h

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <condition_variable>

using namespace std;

class WriterAndReader {
private:
    mutex mut_counter;  // Мьютекс для защиты счетчиков
    condition_variable cv; // Условие для синхронизации
    mutex rw_mutex;    // Мьютекс для управления доступом читателей и писателей
    int nreaders;    // Количество читателей
    int nwriters;    // Количество писателей
    bool writer_active;  // Флаг, указывающий на активность писателя
public:
    WriterAndReader();

    void read(int id, int read_limit);
    void write(int id, int write_limit);
    
    void read_priority();  // Приоритет читателя
    void write_priority(); // Приоритет писателя
};

#endif // RANDW_H
