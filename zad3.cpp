#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>

using namespace std;

class Friend {
public:
    string name;
    int max_need;
    int allocated;
    int need;

    Friend(string n, int max_n, int alloc) : name(n), max_need(max_n), allocated(alloc) {
        need = max_need - allocated;
    }
};

class Bank {
private:
    int available;
    int friend_count;
    Friend** friends; 
    mutex mtx;

public:
    Bank(int avail, int count) : available(avail), friend_count(count) {
        friends = new Friend*[friend_count];
    }

    ~Bank() {
        delete[] friends; 
    }

    void addFriend(int index, Friend* f) {
        if (index < friend_count) {
            friends[index] = f;
        }
    }

    void requestResources(int index, int request) {
        lock_guard<mutex> lock(mtx);
        
        Friend& f = *friends[index];
        cout << f.name << " запрашивает " << request << " рублей.\n";

        if (request <= available && request <= f.need) {
            available -= request;
            f.allocated += request;
            f.need -= request;
            cout << f.name << " получил " << request << " рублей.\n";
            checkSafeState(f);
        } else {
            cout << f.name << " не может получить " << request << " рублей. Недостаточно ресурсов.\n";
        }
    }

    void checkSafeState(Friend &f) {
        // Проверка на безопасное состояние
        int total_allocated = 0;
        int total_needed = 0;

        for (int i = 0; i < friend_count; i++) {
            total_allocated += friends[i]->allocated;
            total_needed += friends[i]->need;
        }

        if (available + total_allocated >= total_needed) {
            cout << "Система в безопасном состоянии.\n";
            available += f.allocated;
            f.allocated = 0;
            cout << f.name << " вернул средства.\n";
        } else {
            cout << "Система не в безопасном состоянии! Возможен тупик.\n";
        }
    }

    bool isDeadlock() {
        for (int i = 0; i < friend_count; i++) {
            if (friends[i]->need > available) {
                return true; // Невозможно удовлетворить потребности хотя бы одного друга
            }
        }
        return false; // Тупика нет
    }

    Friend* getFriend(int index) {
        return (index < friend_count) ? friends[index] : nullptr;
    }
};

int main() {
    int initial_money;
    cout << "Введите начальное количество средств в банке: ";
    cin >> initial_money;

    int friend_count = 3;
    Bank bank(initial_money, friend_count);

    // Инициализация друзей с их максимальными потребностями и выделенными средствами
    bank.addFriend(0, new Friend("Миша", 8, 6));
    bank.addFriend(1, new Friend("Вадим", 13, 8));
    bank.addFriend(2, new Friend("Влад", 10, 7));

    // Изначальное распределение средств
    cout << "\nИзначальное распределение средств:\n";
    for (int i = 0; i < friend_count; ++i) {
        Friend* f = bank.getFriend(i);
        if (f) {
            cout << f->name << " начальные средства: " << f->allocated << " рублей.\n";
        }
    }

    // Проверить общую сумму выделенных средств
    int total_allocated = 0;
    for (int i = 0; i < friend_count; ++i) {
        total_allocated += bank.getFriend(i)->allocated;
    }
    
    if (total_allocated > initial_money) {
        cout << "Ошибка: выделенные средства превышают начальные средства в банке!" << endl;
        return -1; // Завершить программу с ошибкой
    }

    // Запрос ресурсов
    int request;
    for (int i = 0; i < friend_count; i++) {
        cout << "Введите запрашиваемую сумму для " << bank.getFriend(i)->name << ": ";
        cin >> request;
        thread t(&Bank::requestResources, &bank, i, request);
        t.join();
    }

    // Проверка на тупиковую ситуацию
    if (bank.isDeadlock()) {
        cout << "Система находится в тупиковой ситуации!\n";
    }

    for (int i = 0; i < friend_count; ++i) {
        delete bank.getFriend(i);
    }

    return 0;
}