#include "zad2.h"

BirthData::BirthData(size_t size) : size(size) { // Конструктор, выделяющий память для массива родов
    births = new Birth[size];
}

BirthData::~BirthData() { // Деструктор, освобождающий выделенную память
    delete[] births;
}

// Метод для генерации данных о родах
void BirthData::generateData(size_t numEntries) {
    for (size_t i = 0; i < numEntries; ++i) {
        births[i].motherFIO = "Mother" + to_string(i);
        cout << "Введите дату рождения матери для " << births[i].motherFIO << " (ГГГГ-ММ-ДД): ";
        cin >> births[i].birthMother;
        cout << "Введите дату рождения ребенка для " << births[i].motherFIO << " (ГГГГ-ММ-ДД): ";
        cin >> births[i].birthChild;
    }
}

// Метод для разбора даты и извлечения года, месяца и дня
void BirthData::parseDate(const string& date, int& year, int& month, int& day) {
    year = stoi(date.substr(0, 4)); // Извлечение года
    month = stoi(date.substr(5, 2)); // Извлечение месяца
    day = stoi(date.substr(8, 2)); // Извлечение дня
    if (month < 1 || month > 12) {
        throw invalid_argument("Месяц должен быть от 1 до 12.");
    }
    if (year > 2024) {
        throw invalid_argument("Год должен быть не больше 2024.");
    }
}

// Метод проверки, находится ли дата в заданном диапазоне
bool BirthData::DateRange(const string& date, const string& fromDate, const string& toDate) {
    int year, month, day;
    int fromYear, fromMonth, fromDay;
    int toYear, toMonth, toDay;
    parseDate(date, year, month, day);
    parseDate(fromDate, fromYear, fromMonth, fromDay);
    parseDate(toDate, toYear, toMonth, toDay);

    return (year > fromYear || (year == fromYear && 
            (month > fromMonth || (month == fromMonth && day >= fromDay)))) &&
            (year < toYear || (year == toYear && 
            (month < toMonth || (month == toMonth && day <= toDay))));
}

// Метод вычисления среднего возраста матерей в заданном диапазоне дат
double BirthData::calculateAverageAge(const string& fromDate, const string& toDate, size_t start, size_t end) {
    int totalAge = 0; // Сумма возрастов
    int count = 0; // Счетчик подходящих записей

    for (size_t i = start; i < end; ++i) { // Если дата рождения матери попадает в диапазон
        if (DateRange(births[i].birthMother, fromDate, toDate)) {
            totalAge += getAge(births[i].birthMother); // Добавляем возраст
            count++;
        }
    }
    return (count > 0) ? static_cast<double>(totalAge) / count : 0.0; // Возврат среднего возраста
}

int BirthData::getAge(const string& birthDate) { // Метод для вычисления возраста по дате рождения
    int year = stoi(birthDate.substr(0, 4));
    return 2024 - year;  // Логика возраста на 2024 год
}

mutex mtx;

// Функция для расчета среднего возраста в отдельном потоке
void calculateAverageAgeThread(BirthData& data, const string& fromDate, const string& toDate, double& average, size_t start, size_t end) {
    double localAverage = data.calculateAverageAge(fromDate, toDate, start, end);
    lock_guard<mutex> lock(mtx); // Защита от конкурентного доступа
    average += localAverage;  // Обновляем общий средний возраст
}

// Функция для обработки данных в параллельных потоках
void processParallel(BirthData& data, const string& fromDate, const string& toDate, double& result) {
    auto start = chrono::high_resolution_clock::now();

    size_t mid = data.size / 2; // Делим массив пополам
    double average1 = 0;
    double average2 = 0;

    // Создаем два потока для расчета среднего возраста
    thread t1(calculateAverageAgeThread, ref(data), fromDate, toDate, ref(average1), 0, mid);
    thread t2(calculateAverageAgeThread, ref(data), fromDate, toDate, ref(average2), mid, data.size);
    
    t1.join(); // Ожидание завершения первого потока
    t2.join(); // Ожидание завершения второго потока
     
    result = (average1 + average2) / 2.0; // Общий результат как среднее значение двух потоков

    auto end = chrono::high_resolution_clock::now(); // Замер времени окончания
    chrono::duration<double> elapsed = end - start; // Подсчет времени выполнения
    cout << "Время обработки (параллельно): " << elapsed.count() << " секунд\n";
}

// Функция для обработки данных последовательно
void processSequential(BirthData& data, const string& fromDate, const string& toDate, double& result) {
    auto start = chrono::high_resolution_clock::now(); // Начало замера времени
    result = data.calculateAverageAge(fromDate, toDate, 0, data.size); // Последовательный расчет
    auto end = chrono::high_resolution_clock::now(); // Конец замера времени
    chrono::duration<double> elapsed = end - start;
    cout << "Время обработки (последовательно): " << elapsed.count() << " секунд\n";
}

int main() {
    size_t dataSize;
    cout << "Введите количество записей о родах: ";
    cin >> dataSize;
    
    BirthData data(dataSize); // Создаем объект класса BirthData
    data.generateData(dataSize); // Генерируем данные о родах

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