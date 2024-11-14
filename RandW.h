#ifndef RandW_h
#define RandW_h

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

using namespace std;

class WriterAndReader {
private:
    mutex mx;
    condition_variable cv;
    int writers; // кол-во активных писателей
    int readers; // кол-во активный читателей
    bool priority; // выбор приоритета
public:
    WriterAndReader(bool check);

    void startWriting();
    void stopWriting();
    void startReading();
    void stopReading();
    void replacePriority(bool change);
};

#endif // RANDW_H
