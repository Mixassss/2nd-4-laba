#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>

using namespace std;

class Timer {
public:
    Timer() {
        start = chrono::high_resolution_clock::now();
    }

    ~Timer() {
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;
        cout << "Время выполнения: " << duration.count() << " секунд" << endl;
    }

private:
    chrono::high_resolution_clock::time_point start;
};

class Maternity {
public:
    void addRecord() { 
        int zapis;
        cout << "Введите количество записей о родах: ";
        cin >> zapis;

        for (int i = 0; i < zapis; ++i) {
            MaternityRecord record;

            // Генерация даты рождения женщины
            while (true) {
                int day = 1 + rand() % 31;
                int month = 1 + rand() % 12; 
                int year = 1970 + rand() % 26;
                string dateM = to_string(day) + '-' + to_string(month) + '-' + to_string(year);
                if (checkDate(dateM)) {
                    record.womanBirthDate = dateM;
                    break;
                }
            }

            // Генерация даты рождения ребенка
            while (true) {
                int day = 1 + rand() % 31;
                int month = 1 + rand() % 12;
                int year = 2007 + rand() % 18;
                string dateC = to_string(day) + '-' + to_string(month) + '-' + to_string(year);
                if (checkDate(dateC)) {
                    record.babyBirthDate = dateC;
                    break;
                }
            }

            // Генерация ФИО матери
            char motherFIO = 'A' + (rand() % 26); 
            record.motherFIO = string(1, motherFIO);

            records.push_back(record);
        }
    }

    void calculateAverageAge(const string& startDate, const string& endDate, double& averageAge, int& count, int start = 0, int end = -1) {
        if (end == -1) end = records.size();
        double totalAge = 0;
        count = 0;

        for (int i = start; i < end; ++i) {
            const auto& record = records[i];
            if (checkDate(record.babyBirthDate, startDate, endDate)) {
                totalAge += calculateYearDifference(record.womanBirthDate);
                count++;
            }
        }
        
        if (count > 0) {
            averageAge = totalAge / count;
        } else {
            averageAge = 0;
        }
    }

    void printRecordsInRange(const string& startDate, const string& endDate) {
        for (const auto& record : records) {
            if (checkDate(record.babyBirthDate, startDate, endDate)) {
                cout << "ФИО: " << record.motherFIO << " | Дата матери: " << record.womanBirthDate << " | Дата ребенка: " << record.babyBirthDate << endl;
            }
        }
    }

    struct MaternityRecord {
        string womanBirthDate; 
        string babyBirthDate;  
        string motherFIO;
    };

    vector<MaternityRecord> records;

private:
    bool checkDate(const string& date) { 
        stringstream ss(date);
        string token;
        getline(ss, token, '-');
        int day = stoi(token);
        getline(ss, token, '-');
        int month = stoi(token);
        getline(ss, token, '-');
        int year = stoi(token);

        if (month < 1 || month > 12 || day < 1 || day > 31) return false;
        if ((month == 4 || month == 6 || month == 9 || month == 11) && day == 31) return false;
        if (month == 2) {
            bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            return day <= (leap ? 29 : 28);
        }
        return true;
    }

    tm stringToDate(const string& date) {
        int day, month, year;
        stringstream stream(date);
        string token;
        getline(stream, token, '-');
        day = stoi(token);
        getline(stream, token, '-');
        month = stoi(token);
        getline(stream, token, '-');
        year = stoi(token);
        tm dateStruct = {};
        dateStruct.tm_year = year - 1900; 
        dateStruct.tm_mon = month - 1; 
        dateStruct.tm_mday = day; 
        return dateStruct; 
    }

    bool checkDate(const string& babyDate, const string& startDate, const string& endDate) {
        tm babyTm = stringToDate(babyDate);
        tm startTm = stringToDate(startDate);
        tm endTm = stringToDate(endDate);
        
        return difftime(mktime(&babyTm), mktime(&startTm)) >= 0 && difftime(mktime(&endTm), mktime(&babyTm)) >= 0; 
    }

    double calculateYearDifference(const string& birthDate) { 
        tm bDate = stringToDate(birthDate); 
        time_t now = time(nullptr);
        tm* nowTm = localtime(&now);
        double age = difftime(mktime(nowTm), mktime(&bDate)) / (60 * 60 * 24 * 365.25); 
        return age;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0))); 
    Maternity maternity;

    cout << "Выберите действие: 1 - добавить записи, 2 - вычислить средний возраст" << endl;
    while (true) {
        string choice;
        cout << "Вы выбрали: ";
        cin >> choice;

        if (choice == "1") {
            maternity.addRecord();
        } else if (choice == "2") {
            string startDate, endDate;
            cout << "Введите начальную дату (dd-mm-yyyy): ";
            cin >> startDate;
            cout << "Введите конечную дату (dd-mm-yyyy): ";
            cin >> endDate;

            // Однопоточная обработка
            {
                Timer t; 
                double averageAge;
                int count;
                cout << "Однопоточная обработка: " << endl;
                maternity.calculateAverageAge(startDate, endDate, averageAge, count);
                if (count > 0)
                    cout << "Средний возраст: " << averageAge << endl;
                else
                    cout << "Записи не найдены." << endl;

                maternity.printRecordsInRange(startDate, endDate);
            }

            // Многопоточная обработка
            int countThreads;
            cout << "Введите количество потоков: ";
            cin >> countThreads;

            int size = maternity.records.size(); 
            int chunkSize = (size + countThreads - 1) / countThreads; 
            vector<thread> threads(countThreads); 
            vector<double> averages(countThreads, 0); 
            vector<int> counts(countThreads, 0);

            Timer t;
            cout << "Многопоточная обработка: " << endl;
            for (int i = 0; i < countThreads; ++i) {
                int start = i * chunkSize;
                int end = min(start + chunkSize, size);
                if (start < size) { // Проверка, чтобы избежать пустых потоков
                    threads[i] = thread([&maternity, &startDate, &endDate, i, start, end, &averages, &counts]() {
                        maternity.calculateAverageAge(startDate, endDate, averages[i], counts[i], start, end);
                    });
                }
            }
            for (int i = 0; i < countThreads; ++i) {
                if (threads[i].joinable()) {
                    threads[i].join();
                }
            }

            double totalAge = 0;
            int totalCount = 0;
            for (int i = 0; i < countThreads; ++i) {
                totalAge += averages[i] * counts[i];
                totalCount += counts[i];
            }
            if (totalCount > 0)
                cout << "Средний возраст: " << totalAge / totalCount << endl;
            else
                cout << "Записи не найдены." << endl;

            maternity.printRecordsInRange(startDate, endDate);
            break;
        }
    }
    return 0;
}
