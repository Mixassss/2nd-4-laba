#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm> // Для std::min

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
    void addRecord() { // Метод для добавления записей
        int zapis;
        cout << "Введите количество записей о родах: ";
        cin >> zapis; // Ввод количества записей

        for (int i = 0; i < zapis; ++i) { // Цикл для добавления записей
            MaternityRecord record;

            while (true) { // Генерация даты рождения женщины
                int day = 1 + rand() % 31; // Случайный день
                int month = 1 + rand() % 12; // Случайный месяц
                int year = 1970 + rand() % 26; // Случайный год
                string dateM = to_string(day) + '-' + to_string(month) + '-' + to_string(year);
                if (checkDate(dateM)) { // Проверка корректности даты
                    record.womanBirthDate = dateM; // Сохранение даты
                    break;
                }
            }

            while (true) { // Генерация даты рождения ребенка
                int day = 1 + rand() % 31; // Случайный день
                int month = 1 + rand() % 12; // Случайный месяц
                int year = 2007 + rand() % 18; // Случайный год
                string dateC = to_string(day) + '-' + to_string(month) + '-' + to_string(year);
                if (checkDate(dateC)) { // Проверка корректности даты
                    record.babyBirthDate = dateC; // Сохранение даты
                    break;
                }
            }

            // Генерация ФИО матери
            char motherFIO = 'A' + (rand() % 26); 
            record.motherFIO = string(1, motherFIO);

            records.push_back(record); // Добавление записи в вектор
        }
    }

    // Метод для вычисления среднего возраста
    void calculateAverageAge(const string& startDate, const string& endDate, double& averageAge, int& count, int start = 0, int end = -1) {
        if (end == -1) end = records.size(); // Если end не задан, берем размер вектора
        double totalAge = 0;
        count = 0;

        for (int i = start; i < end; ++i) { // Перебор всех записей
            const auto& record = records[i];
            if (checkDate(record.babyBirthDate, startDate, endDate)) { // Проверка даты рождения ребенка
                totalAge += calculateYearDifference(record.womanBirthDate); // Суммирование возрастов
                count++; // Увеличение счетчика
            }
        }
        
        if (count > 0) {
            averageAge = totalAge / count; // Вычисление среднего возраста
        } else {
            averageAge = 0; // Если записей нет, средний возраст 0
        }
    }

    // Метод для вывода записей в заданном диапазоне дат
    void printRecordsInRange(const string& startDate, const string& endDate) {
        for (const auto& record : records) { // Перебор записей
            if (checkDate(record.babyBirthDate, startDate, endDate)) { // Проверка даты
                cout << "ФИО: " << record.motherFIO << " | Дата матери: " << record.womanBirthDate << " | Дата ребенка: " << record.babyBirthDate << endl;
            }
        }
    }

    struct MaternityRecord { // Структура для записи о родах
        string womanBirthDate; // Дата рождения матери
        string babyBirthDate; // Дата рождения ребенка
        string motherFIO; // ФИО матери
    };

    vector<MaternityRecord> records; // Вектор для хранения записей

private:
    bool checkDate(const string& date) { // Метод для проверки корректности формата даты
        stringstream ss(date);
        string token;
        getline(ss, token, '-');
        int day = stoi(token);
        getline(ss, token, '-');
        int month = stoi(token);
        getline(ss, token, '-');
        int year = stoi(token);

        if (month < 1 || month > 12 || day < 1 || day > 31) return false; // Проверка диапазона
        if ((month == 4 || month == 6 || month == 9 || month == 11) && day == 31) return false; // Месяцы с 30 днями
        if (month == 2) {
            bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0); // Проверка на високосный год
            return day <= (leap ? 29 : 28);
        }
        return true; // Дата корректна
    }

    tm stringToDate(const string& date) { // Преобразование строки в структуру даты
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
        dateStruct.tm_year = year - 1900; // Корректировка года
        dateStruct.tm_mon = month - 1; // Корректировка месяца
        dateStruct.tm_mday = day; // Сохранение дня
        return dateStruct; 
    }

    bool checkDate(const string& babyDate, const string& startDate, const string& endDate) { // Проверка, попадает ли дата ребенка в диапазон
        tm babyTm = stringToDate(babyDate); // Преобразование даты ребенка
        tm startTm = stringToDate(startDate); // Преобразование начальной даты
        tm endTm = stringToDate(endDate); // Преобразование конечной даты
        
        return difftime(mktime(&babyTm), mktime(&startTm)) >= 0 && difftime(mktime(&endTm), mktime(&babyTm)) >= 0; 
    }

    double calculateYearDifference(const string& birthDate) {  // Метод для вычисления возрастной разницы
        tm bDate = stringToDate(birthDate); 
        time_t now = time(nullptr);
        tm* nowTm = localtime(&now);
        double age = difftime(mktime(nowTm), mktime(&bDate)) / (60 * 60 * 24 * 365.25); // Вычисление возраста
        return age;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0))); // Инициализация генератора случайных чисел
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
                maternity.calculateAverageAge(startDate, endDate, averageAge, count); // Вычисление среднего возраста
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
            int chunkSize = (size + countThreads - 1) / countThreads; // Вычисление размера блока для каждого потока
            vector<thread> threads(countThreads); // Вектор для хранения потоков
            vector<double> averages(countThreads, 0); // Вектор для хранения средних возрастов
            vector<int> counts(countThreads, 0); // Вектор для хранения счетчиков

            Timer t;
            cout << "Многопоточная обработка: " << endl;
            for (int i = 0; i < countThreads; ++i) {
                int start = i * chunkSize; // Начало блока
                int end = min(start + chunkSize, size); // Конец блока
                if (start < size) { // Проверка, чтобы избежать пустых потоков
                    threads[i] = thread([&maternity, &startDate, &endDate, i, start, end, &averages, &counts]() {
                        maternity.calculateAverageAge(startDate, endDate, averages[i], counts[i], start, end); // Вызов метода в потоке
                    });
                }
            }
            for (int i = 0; i < countThreads; ++i) {
                if (threads[i].joinable()) {
                    threads[i].join(); // Ожидание завершения потоков
                }
            }

            double totalAge = 0; // Общий возраст
            int totalCount = 0; // Общее количество записей
            for (int i = 0; i < countThreads; ++i) { // Суммирование результатов
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
