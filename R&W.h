#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

using namespace std;

class WriterAndReader {
private:
    mutex no_reads;    // Защита от читателей, когда писатель хочет писать
    mutex no_writes;   // Защита от писателей, когда читатели читают
    mutex mut_counter; // Защита счетчика читателей
    atomic<int> nreaders;  // Количество активных читателей
public:
    WriterAndReader();

    void read(int id, int read_limit);
    void write(int id, int write_limit);
};