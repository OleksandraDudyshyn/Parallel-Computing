#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>
#include <cmath>
#include <chrono>
#include <deque>
#include <functional>
#include <windows.h>

using namespace std;

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int nthPrime(int n) {
    int count = 0;
    int num = 1;
    while (count < n) {
        num++;
        if (isPrime(num)) count++;
    }
    return num;
}

// Завдання 1.2.1
queue<int> q1;
mutex m1;
bool flag1 = false;

void DataPreparation1() {
    int val;
    cout << "Вводьте числа для черги (0 для завершення): ";
    while (cin >> val && val != 0) {
        lock_guard<mutex> lk(m1);
        q1.push(val);
    }
    lock_guard<mutex> lk(m1);
    flag1 = true;
}

void DataProcessing1() {
    unique_lock<mutex> lk(m1);
    while (!flag1) {
        lk.unlock();
        this_thread::sleep_for(chrono::milliseconds(100));
        lk.lock();
    }
    cout << "Прості числа з черги: ";
    while (!q1.empty()) {
        int val = q1.front();
        q1.pop();
        if (isPrime(val)) cout << val << " ";
    }
    cout << "\n";
}

// Завдання 1.2.2 та 1.2.3
int cv_i = 0;
condition_variable cv2;
mutex m2;

void Waits(int threadNum) {
    cout << "Потік " << threadNum << " входить у стан очікування.\n";
    unique_lock<mutex> lk(m2);
    cv2.wait(lk, [] { return cv_i == 1; });
    cout << "Очікування завершене. Повідомлення з потоку " << threadNum << "\n";
}

void Awake() {
    this_thread::sleep_for(chrono::milliseconds(100));
    cout << "Починаю повідомляти решту...\n";
    cv2.notify_all();
    this_thread::sleep_for(chrono::milliseconds(100));

    lock_guard<mutex> lk(m2);
    cv_i = 1;
    cout << "Повторне повідомлення...\n";
    cv2.notify_all();
}

void NotifyThread() {
    this_thread::sleep_for(chrono::milliseconds(100));
    lock_guard<mutex> lk(m2);
    cv_i = 1;
    cv2.notify_one();
}

// Завдання 1.2.4
queue<int> q4;
mutex m4;
condition_variable cv4;
bool done4 = false;

void DataPreparation4() {
    int val;
    cout << "Вводьте числа (0 для завершення): ";
    while (cin >> val && val != 0) {
        lock_guard<mutex> lk(m4);
        q4.push(val);
        cv4.notify_one();
    }
    lock_guard<mutex> lk(m4);
    done4 = true;
    cv4.notify_one();
}

void DataProcessing4() {
    unique_lock<mutex> lk(m4);
    cv4.wait(lk, [] { return !q4.empty() || done4; });
    cout << "Прості числа (умовні змінні): ";
    while (!q4.empty() || !done4) {
        if (q4.empty()) {
            cv4.wait(lk, [] { return !q4.empty() || done4; });
        }
        if (!q4.empty()) {
            int val = q4.front();
            q4.pop();
            lk.unlock();
            if (isPrime(val)) cout << val << " ";
            lk.lock();
        }
    }
    cout << "\n";
}

// Завдання 1.2.7
void PromiseThread1(int n, promise<int>& p1, promise<bool>& pBool, promise<int>& p2) {
    p1.set_value(nthPrime(n));
    pBool.set_value(true);
    p2.set_value(nthPrime(n * 10));
}

void PromiseThread2(int n, future<bool>& fBool) {
    fBool.wait();
    this_thread::sleep_for(chrono::seconds(2));
    cout << "Квадратний корінь з n: " << sqrt(n) << "\n";
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    cout << " Завдання 1.2.1 \n";
    thread t1_prep(DataPreparation1);
    t1_prep.detach();
    DataProcessing1();

    cout << "\n Завдання 1.2.2 \n";
    cv_i = 0;
    thread t2_w(Waits, 0);
    thread t2_a(Awake);
    t2_a.join();
    t2_w.join();

    cout << "\n Завдання 1.2.3 \n";
    cv_i = 0;
    thread t3_1(Waits, 1);
    thread t3_2(Waits, 2);
    thread t3_3(Waits, 3);
    thread t3_n(NotifyThread);
    t3_n.join();
    cv_i = 1;
    cv2.notify_all();
    t3_1.join();
    t3_2.join();
    t3_3.join();

    cout << "\n Завдання 1.2.4 \n";
    thread t4_prep(DataPreparation4);
    thread t4_proc(DataProcessing4);
    t4_prep.join();
    t4_proc.join();

    cout << "\n Завдання 1.2.5 \n";
    int n;
    cout << "Введіть номер простого числа (n): ";
    cin >> n;
    auto f_def = async(launch::deferred, nthPrime, n);
    cout << "Корінь з n: " << sqrt(n) << "\n";
    cout << "Результат (deferred): " << f_def.get() << "\n";

    auto f_async = async(launch::async, nthPrime, n);
    cout << "Логарифм з n: " << log(n) << "\n";
    cout << "Результат (async): " << f_async.get() << "\n";

    cout << "\n Завдання 1.2.6 \n";
    deque<packaged_task<int()>> tasks;
    deque<int> n_values;
    mutex d_mut;
    bool stopTasks = false;

    thread worker([&]() {
        while (true) {
            packaged_task<int()> task;
            int current_n = 0;
            {
                lock_guard<mutex> lk(d_mut);
                if (tasks.empty()) {
                    if (stopTasks) break;
                    continue;
                }
                task = move(tasks.front());
                tasks.pop_front();
                current_n = n_values.front();
                n_values.pop_front();
            }
            task();
        }
        });

    cout << "Вводьте n для обчислення (0 для стоп): ";
    int val6;
    vector<future<int>> results6;
    while (cin >> val6 && val6 != 0) {
        packaged_task<int()> task(bind(nthPrime, val6));
        results6.push_back(task.get_future());
        lock_guard<mutex> lk(d_mut);
        tasks.push_back(move(task));
        n_values.push_back(val6);
    }
    {
        lock_guard<mutex> lk(d_mut);
        stopTasks = true;
    }
    worker.join();
    for (auto& res : results6) {
        cout << "Знайдене просте число: " << res.get() << "\n";
    }

    cout << "\n Завдання 1.2.7 \n";
    int n7;
    cout << "Введіть n: ";
    cin >> n7;
    promise<int> p1, p2;
    promise<bool> pBool;
    future<int> f1 = p1.get_future();
    future<int> f2 = p2.get_future();
    future<bool> fBool = pBool.get_future();

    thread pt1(PromiseThread1, n7, ref(p1), ref(pBool), ref(p2));
    thread pt2(PromiseThread2, n7, ref(fBool));

    cout << n7 << "-те просте число: " << f1.get() << "\n";
    cout << n7 * 10 << "-те просте число: " << f2.get() << "\n";

    pt1.join();
    pt2.join();

    return 0;
}