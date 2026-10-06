#include <iostream>
#include <thread>
#include <list>
#include <mutex>
#include <algorithm>
#include <string>
#include <windows.h>

using namespace std;

void Thread1() {
    cout << "1" << endl;
}

void Thread2() {
    cout << "2" << endl;
}

list<int> globalList;
mutex listMutex;

void AddToList(int startValue) {
    lock_guard<mutex> guard(listMutex);
    for (int i = 0; i < 10; ++i) {
        int newValue = startValue + i;
        globalList.push_back(newValue);
        cout << "AddToList додано " << newValue << endl;
    }
}

void ListContains(int checkValue) {
    lock_guard<mutex> guard(listMutex);
    for (int i = 0; i < 10; ++i) {
        bool found = (find(globalList.begin(), globalList.end(), checkValue) != globalList.end());
        if (found) {
            cout << "ListContains елемент " << checkValue << " знайдено." << endl;
        }
        else {
            cout << "ListContains елемент " << checkValue << " не знайдено." << endl;
        }
    }
}

class someData {
public:
    string firstName;
    string lastName;
    string address;
    int age;

    someData(string fName, string lName, string addr, int a)
        : firstName(fName), lastName(lName), address(addr), age(a) {
    }

    void print() {
        cout << firstName << " " << lastName << " " << address << " Age " << age << endl;
    }
};

class exchangePerson {
public:
    someData data;
    mutex m;

    exchangePerson(someData d) : data(d) {}

    static void JohnDoe(exchangePerson& p) {
        lock_guard<mutex> guard(p.m);
        p.data.firstName = "John";
        p.data.lastName = "Doe";
        p.data.address = "Unknown";
        p.data.age = 120;
    }

    static void JacobSmith(exchangePerson& p) {
        lock_guard<mutex> guard(p.m);
        p.data.firstName = "Jacob";
        p.data.lastName = "Smith";
        p.data.address = "Known";
        p.data.age = 1;
    }

    static void Swap(exchangePerson& p1, exchangePerson& p2) {
        if (&p1 == &p2) return;

        unique_lock<mutex> lock1(p1.m, defer_lock);
        unique_lock<mutex> lock2(p2.m, defer_lock);

        lock(lock1, lock2);

        cout << "\nПеред обмiном \n";
        cout << "P1 "; p1.data.print();
        cout << "P2 "; p2.data.print();

        swap(p1.data, p2.data);

        cout << "\nПiсля обмiну \n";
        cout << "P1 "; p1.data.print();
        cout << "P2 "; p2.data.print();
    }
};

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    cout << "Завдання 1.2.1 та 1.2.2\n";
    thread t1(Thread1);
    thread t2(Thread2);
    t1.join();
    t2.join();

    cout << "\nЗавдання 1.2.3, 1.2.4, 1.2.5\n";
    thread threads[20];
    int startVal = 1;
    int checkVal = 5;

    for (int i = 0; i < 10; ++i) {
        threads[i * 2] = thread(AddToList, startVal);
        threads[i * 2 + 1] = thread(ListContains, checkVal);
        startVal++;
    }

    for (int i = 0; i < 20; ++i) {
        threads[i].detach();
    }

    this_thread::sleep_for(chrono::milliseconds(500));

    cout << "\nЗавдання 1.2.6 та 1.2.7\n";
    exchangePerson person1(someData("Initial1", "Last1", "Addr1", 20));
    exchangePerson person2(someData("Initial2", "Last2", "Addr2", 30));

    thread tj1(exchangePerson::JohnDoe, ref(person1));
    thread tj2(exchangePerson::JacobSmith, ref(person2));
    tj1.detach();
    tj2.detach();

    this_thread::sleep_for(chrono::milliseconds(100));

    thread tSwap(exchangePerson::Swap, ref(person1), ref(person2));
    tSwap.join();

    return 0;
}