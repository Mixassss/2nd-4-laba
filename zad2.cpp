#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <cmath>
#include <iomanip>
#include <mutex>

using namespace std;

struct Birth {
    string motherFIO;
    string birthMother;
    string birthChild;
};

class BirthData {
public:
    Birth *births;
    size_t size;

    BirthData(size_t size) : size(size) {
        births = new Birth[size];
    }

    ~BirthData() {
        delete[] births;
    }

    void generateDate() {
        for (size_t i = 0; i < size; ++i) {
            births[i].motherFIO = "Mother" + to_string(i);
            births[i].birthMother = "1980-01-01";
            births[i].birthChild = "2023-01-01";
        }
    }

    double calculateAverageAge(const string& fromDate, const string& toDate) {
        int totalAge = 0;
        int count = 0;

        for (size_t i = 0; i < size; ++i) {
            if (births[i].birthMother >= fromDate && births[i].birthMother <= toDate) {
                totalAge += getAge(births[i].birthMother);
                count++;
            }
        }
        return (count > 0) ? static_cast<double>(totalAge) / count : 0.0;
    }

private:
    int getAge(const string& birthDate) {
        return 2024 - stoi(birthDate.substr(0, 4));
    }
};

mutex mtx;

void processSequential(BirthData& data, const string& fromDate, const string& toDate, double& result) {
    auto start = chrono::high_resolution_clock::now();
    result = data.calculateAverageAge(fromDate, toDate);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Время обработки (последовательно): " << elapsed.count() << " секунд\n";
}

void processParallel(BirthData& data, const string& fromDate, const string& toDate, double& result) {
    auto start = chrono::high_resolution_clock::now();
    
    size_t mid = data.size / 2;
    double average1 = 0;
    double average2 = 0;

    auto future1 = async(launch::async, [&data, &fromDate, &toDate, mid, &average1]() {
        double localResult = data.calculateAverageAge(fromDate, toDate);
        lock_guard<mutex> lock(mtx);
        average1 = localResult;
    });
    
    auto future2 = async(launch::async, [&data, &fromDate, &toDate, mid, &average2]() {
        BirthData subArray(mid);
        for (size_t i = 0; i < mid; ++i)
            subArray.births[i] = data.births[i];
        double localResult = subArray.calculateAverageAge(fromDate, toDate);
        lock_guard<mutex> lock(mtx);
        average2 = localResult;
    });

    future1.get();
    future2.get();
    result = (average1 + average2) / 2;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Время обработки (параллельно): " << elapsed.count() << " секунд\n";
}

int main() {
    const size_t dataSize = 100000;
    BirthData data(dataSize);
    data.generateDate();

    string fromDate = "1980-01-01";
    string toDate = "2000-01-01";
    double sequentialResult, parallelResult;

    processSequential(data, fromDate, toDate, sequentialResult);
    processParallel(data, fromDate, toDate, parallelResult);

    cout << fixed << setprecision(2);
    cout << "Средний возраст (последовательно): " << sequentialResult << "\n";
    cout << "Средний возраст (параллельно): " << parallelResult << "\n";

    return 0;
}