#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

class MaternityRecord {
public:
    string womanName;      // ФИО женщины
    string womanBirthDate; // Дата рождения женщины (dd-mm-yyyy)
    string babyBirthDate;  // Дата рождения ребенка (dd-mm-yyyy)
};

class MaternityData {
public:
    void addRecord() {
        int n;
        cout << "Введите количество записей о родах: ";
        cin >> n;

        for (int i = 0; i < n; ++i) {
            MaternityRecord record;
            record.womanBirthDate = generateRandomDate(1975, 2000); // Генерация даты рождения женщины
            record.babyBirthDate = generateRandomDate(2005, 2024);   // Генерация даты рождения ребенка
            records.push_back(record);
        }
    }

    double calculateAverageAge(const string& startDate, const string& endDate) {
        double totalAge = 0;
        int count = 0;
        tm startTm = stringToDate(startDate);
        tm endTm = stringToDate(endDate);

        for (const auto& record : records) {
            tm babyTm = stringToDate(record.babyBirthDate);
            if (isDateInRange(babyTm, startTm, endTm)) {
                int age = calculateYearDifference(record.womanBirthDate);
                totalAge += age;
                count++;
            }
        }
        return (count > 0) ? totalAge / count : 0;
    }

    void processRecords(int startIdx, int endIdx, double &averageAge, const string& startDate, const string& endDate) {
        double ageSum = 0;
        int count = 0;

        for (int i = startIdx; i < endIdx; i++) {
            int age = calculateYearDifference(records[i].womanBirthDate);
            tm babyTm = stringToDate(records[i].babyBirthDate);
            if (isDateInRange(babyTm, stringToDate(startDate), stringToDate(endDate))) {
                ageSum += age;
                count++;
            }
        }

        lock_guard<mutex> guard(mx);
        if (count > 0) {
            averageAge += ageSum;
            averageCount += count;
        }
    }

    vector<MaternityRecord> records;
private:
    mutex mx;
    int averageCount = 0;

    string generateRandomDate(int startYear, int endYear) {
        int day, month, year;
        while (true) {
            day = rand() % 31 + 1; // 1-31
            month = rand() % 12 + 1; // 1-12
            year = rand() % (endYear - startYear + 1) + startYear; // Ограниченный диапазон годов

            if (checkDate(day, month, year)) {
                string date = to_string(day) + '-' + to_string(month) + '-' + to_string(year);
                return date;
            }
        }
    }

    int calculateYearDifference(const string& birthDate) {
        int day, month, year;
        stringstream ss(birthDate);
        string token;

        getline(ss, token, '-');
        day = stoi(token);
        getline(ss, token, '-');
        month = stoi(token);
        getline(ss, token, '-');
        year = stoi(token);

        time_t now = time(0);
        tm *ltm = localtime(&now);
        int currentYear = ltm->tm_year + 1900;
        return currentYear - year;
    }

    tm stringToDate(const string& date) {
        tm tm = {};
        stringstream ss(date);
        string token;

        getline(ss, token, '-');
        tm.tm_mday = stoi(token);
        getline(ss, token, '-');
        tm.tm_mon = stoi(token) - 1; // Месяцы начинаются с 0
        getline(ss, token, '-');
        tm.tm_year = stoi(token) - 1900; // Год с 1900

        return tm;
    }

    bool isDateInRange(const tm& date, const tm& start, const tm& end) {
        time_t dateTime = mktime(const_cast<tm*>(&date));
        time_t startTime = mktime(const_cast<tm*>(&start));
        time_t endTime = mktime(const_cast<tm*>(&end));

        return (dateTime >= startTime && dateTime <= endTime);
    }

    bool checkDate(int day, int month, int year) {
        if (month < 1 || month > 12 || day < 1 || day > 31)
            return false;

        if ((month == 4 || month == 6 || month == 9 || month == 11) && day == 31)
            return false; // Апрель, Июнь, Сентябрь, Ноябрь

        if (month == 2) {
            bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (day > (leap ? 29 : 28))
                return false; // Февраль
        }
        return true;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0)));
    MaternityData data;

    cout << "Выберите действие: 1-добавить записи, 2-вычислить средний возраст, 3-выход" << endl;
    while (true) {
        string choice;
        cout << "Ваш выбор: ";
        cin >> choice;

        if (choice == "1") {
            data.addRecord();
        } else if (choice == "2") {
            string startDate, endDate;
            cout << "Введите начальную дату (dd-mm-yyyy): ";
            cin >> startDate;
            cout << "Введите конечную дату (dd-mm-yyyy): ";
            cin >> endDate;

            cout << "Однопоточная обработка: " << endl;
            auto start = chrono::high_resolution_clock::now();
            double averageAgeSingle = data.calculateAverageAge(startDate, endDate);
            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsedSingle = end - start;
            cout << "Средний возраст: " << averageAgeSingle << endl;
            cout << "Время однопоточной обработки: " << elapsedSingle.count() << " секунд" << endl;

            int countThreads;
            cout << "Введите количество потоков: ";
            cin >> countThreads;

            int size = data.records.size();
            vector<thread> threads(countThreads);
            double totalAge = 0;
            int totalCount = 0;
            int chunkSize = size / countThreads;

            auto startMulti = chrono::high_resolution_clock::now();
            for (int i = 0; i < countThreads; ++i) {
                int startIdx = i * chunkSize;
                int endIdx = (i == countThreads - 1) ? size : startIdx + chunkSize;
                threads[i] = thread(&MaternityData::processRecords, &data, startIdx, endIdx, ref(totalAge), startDate, endDate);
            }
            for (auto& t : threads) {
                t.join();
            }
            double averageAgeMulti = (totalCount > 0) ? totalAge / totalCount : 0;
            auto endMulti = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsedMulti = endMulti - startMulti;

            cout << "Средний возраст в многопоточном режиме: " << averageAgeMulti << endl;
            cout << "Время многопоточной обработки: " << elapsedMulti.count() << " секунд" << endl;
        } else if (choice == "3") {
            cout << "Выход..." << endl;
            break;
        } else {
            cout << "Ошибка, нет такого действия!" << endl;
        }
    }

    return 0;
}
