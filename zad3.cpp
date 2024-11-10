#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

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
    Friend** friends; // Динамический массив указателей на друзей
    mutex mtx;

public:
    Bank(int avail, int count) : available(avail), friend_count(count) {
        friends = new Friend*[friend_count];
    }

    ~Bank() {
        delete[] friends; // Освобождение памяти
    }

    void addFriend(int index, Friend* f) {
        if (index < friend_count) {
            friends[index] = f;
        }
    }

    void requestResources(Friend &f, int request) {
        lock_guard<mutex> lock(mtx);
        
        if (request <= available && request <= f.need) {
            // Предполагаем, что ресурсы выделяются
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
            // Вернуть ресурсы
            available += f.allocated;
            f.allocated = 0;
            cout << f.name << " вернул средства.\n";
        } else {
            cout << "Система не в безопасном состоянии! Возможен тупик.\n";
        }
    }

};

int main() {
    Bank bank(3, 3); // общий доступный ресурс и количество друзей

    Friend* chandler = new Friend("Чендлер", 8, 6);
    Friend* ross = new Friend("Росс", 13, 8);
    Friend* joey = new Friend("Джоуи", 10, 7);

    bank.addFriend(0, chandler);
    bank.addFriend(1, ross);
    bank.addFriend(2, joey);

    thread t1(&Bank::requestResources, &bank, ref(*chandler), 2); // Чендлер запрашивает 2 доллара
    thread t2(&Bank::requestResources, &bank, ref(*ross), 5);     // Росс запрашивает 5 долларов
    thread t3(&Bank::requestResources, &bank, ref(*joey), 3);    // Джоуи запрашивает 3 доллара

    t1.join();
    t2.join();
    t3.join();

    // Освобождение памяти
    delete chandler;
    delete ross;
    delete joey;

    return 0;
}