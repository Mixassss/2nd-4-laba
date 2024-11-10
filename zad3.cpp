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
    int initial_money;

    Friend(string n, int max_n, int alloc, int init) : name(n), max_need(max_n), allocated(alloc), initial_money(init) {
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

    void requestResources(Friend &f, int request) {
        lock_guard<mutex> lock(mtx);
        
        cout << f.name << " запрашивает " << request << " долларов.\n";

        if (request <= available && request <= f.need) {
            available -= request;
            f.allocated += request;
            f.need -= request;
            cout << f.name << " получил " << request << " долларов.\n";

            this_thread::sleep_for(chrono::seconds(1));
            checkSafeState(f);
        } else {
            cout << f.name << " не может получить " << request << " долларов. Недостаточно ресурсов.\n";
        }
    }

    void checkSafeState(Friend &f) {
        int total_allocated = 0;
        for (int i = 0; i < friend_count; i++) {
            total_allocated += friends[i]->allocated;
        }
        
        int total_needed = 0;
        for (int i = 0; i < friend_count; i++) {
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

    int friend_count;
    cout << "Введите количество друзей: ";
    cin >> friend_count;

    Bank bank(initial_money, friend_count);
    
    string name;
    int max_need, alloc, init;
    for (int i = 0; i < friend_count; ++i) {
        cout << "Введите имя друга, максимальную потребность, выделенные средства и начальные деньги (через пробел): ";
        cin >> name >> max_need >> alloc >> init;
        Friend* friendEntity = new Friend(name, max_need, alloc, init);
        bank.addFriend(i, friendEntity);
    }

    // Изначальное распределение средств
    cout << "\nИзначальное распределение средств:\n";
    for (int i = 0; i < friend_count; ++i) {
        Friend* f = bank.getFriend(i);
        if (f) {
            cout << f->name << " начальные средства: " << f->initial_money << " долларов.\n";
        }
    }

    thread t1(&Bank::requestResources, &bank, ref(*bank.getFriend(0)), 6); 
    thread t2(&Bank::requestResources, &bank, ref(*bank.getFriend(1)), 8);     
    thread t3(&Bank::requestResources, &bank, ref(*bank.getFriend(2)), 7);   

    // Проверка на тупиковую ситуацию
    if (bank.isDeadlock()) {
        cout << "Система находится в тупиковой ситуации!\n";
    }

    t1.join();
    t2.join();
    t3.join();

    for (int i = 0; i < friend_count; ++i) {
        delete bank.getFriend(i);
    }

    return 0;
}