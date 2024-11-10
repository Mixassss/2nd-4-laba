#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <iomanip>

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

    void generateData(size_t numEntries) {
        for (size_t i = 0; i < numEntries; ++i) {
            births[i].motherFIO = "Mother" + to_string(i);
            cout << "Введите дату рождения матери для " << births[i].motherFIO << " (ГГГГ-ММ-ДД): ";
            cin >> births[i].birthMother;
            cout << "Введите дату рождения ребенка для " << births[i].motherFIO << " (ГГГГ-ММ-ДД): ";
            cin >> births[i].birthChild;
        }
    }

    void parseDate(const string& date, int& year, int& month, int& day) {
        year = stoi(date.substr(0, 4));
        month = stoi(date.substr(5, 2));
        day = stoi(date.substr(8, 2));
        if (month < 1 || month > 12) {
            throw invalid_argument("Месяц должен быть от 1 до 12.");
        }
        if (year > 2024) {
            throw invalid_argument("Год должен быть не больше 2024.");
        }
    }

    bool isWithinDateRange(const string& date, const string& fromDate, const string& toDate) {
        int year, month, day;
        int fromYear, fromMonth, fromDay;
        int toYear, toMonth, toDay;
        parseDate(date, year, month, day);
        parseDate(fromDate, fromYear, fromMonth, fromDay);
        parseDate(toDate, toYear, toMonth, toDay);

        return (year > fromYear || (year == fromYear && (month > fromMonth || (month == fromMonth && day >= fromDay)))) &&
               (year < toYear || (year == toYear && (month < toMonth || (month == toMonth && day <= toDay))));
    }

    double calculateAverageAge(const string& fromDate, const string& toDate, size_t start, size_t end) {
        int totalAge = 0;
        int count = 0;

        for (size_t i = start; i < end; ++i) {
            if (isWithinDateRange(births[i].birthMother, fromDate, toDate)) {
                totalAge += getAge(births[i].birthMother);
                count++;
            }
        }
        return (count > 0) ? static_cast<double>(totalAge) / count : 0.0;
    }

private:
    int getAge(const string& birthDate) {
        int year = stoi(birthDate.substr(0, 4));
        return 2024 - year;  // Логика возраста на 2024 год
    }
};

mutex mtx;

void calculateAverageAgeThread(BirthData& data, const string& fromDate, const string& toDate, double& average, size_t start, size_t end) {
    double localAverage = data.calculateAverageAge(fromDate, toDate, start, end);
    lock_guard<mutex> lock(mtx);
    average += localAverage;
}

void processParallel(BirthData& data, const string& fromDate, const string& toDate, double& result) {
    auto start = chrono::high_resolution_clock::now();

    size_t mid = data.size / 2;
    double average1 = 0;
    double average2 = 0;

    thread thread1(calculateAverageAgeThread, ref(data), fromDate, toDate, ref(average1), 0, mid);
    thread thread2(calculateAverageAgeThread, ref(data), fromDate, toDate, ref(average2), mid, data.size);
    
    thread1.join();
    thread2.join();
    
    result = (average1 + average2) / 2.0;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Время обработки (параллельно): " << elapsed.count() << " секунд\n";
}

void processSequential(BirthData& data, const string& fromDate, const string& toDate, double& result) {
    auto start = chrono::high_resolution_clock::now();
    result = data.calculateAverageAge(fromDate, toDate, 0, data.size);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Время обработки (последовательно): " << elapsed.count() << " секунд\n";
}

int main() {
    size_t dataSize;
    cout << "Введите количество записей о родах: ";
    cin >> dataSize;
    
    BirthData data(dataSize);
    data.generateData(dataSize);

    string fromDate, toDate;
    cout << "Введите дату начала (ГГГГ-ММ-ДД): ";
    cin >> fromDate;
    cout << "Введите дату конца (ГГГГ-ММ-ДД): ";
    cin >> toDate;

    double sequentialResult, parallelResult;

    processSequential(data, fromDate, toDate, sequentialResult);
    processParallel(data, fromDate, toDate, parallelResult);

    cout << "Средний возраст (последовательно): " << sequentialResult << "\n";
    cout << "Средний возраст (параллельно): " << parallelResult << "\n";

    return 0;
}