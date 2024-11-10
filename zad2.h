#ifndef zad2_h
#define zad2_h

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <iomanip>

using namespace std;

// Структура для хранения данных о родах
struct Birth {
    string motherFIO;
    string birthMother; // Дата рождения матери
    string birthChild; // Дата рождения ребенка
};

// Класс для обработки данных о родах
class BirthData {
public:
    Birth *births;
    size_t size;

    BirthData(size_t size);
    ~BirthData();

    void generateData(size_t numEntries);
    void parseDate(const string& date, int& year, int& month, int& day);
    bool DateRange(const string& date, const string& fromDate, const string& toDate);
    double calculateAverageAge(const string& fromDate, const string& toDate, size_t start, size_t end);
private:
    int getAge(const string& birthDate);
};

#endif // ZAD2_H