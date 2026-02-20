#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

class Account {

    double balance_;
    string id_;
    mutable mutex mtx_;

    friend class TransactionManager;

    void add(double amount) { balance_ += amount; }
    bool sub(double amount) {
        if (balance_ < amount) return false;
        balance_ -= amount;
        return true;
    }


public:
    Account(string id, double balance): id_(id), balance_(balance) {}

    string get_id() const { return id_; }

    double getBalance() const {
        std::lock_guard<mutex> lock(mtx_);
        return balance_;
    }
};

struct Transaction {
    enum class Type{Deposit, Withdraw};
    Type type{};
    string account_id;
    double amount{};
    double before{};
    double after{};
    thread::id tid{};
    long long timestamp{};
    bool ok{true};
};

class TransactionManager {
    TransactionManager() = default;

    static long long nowMs() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    mutable std::mutex log_mtx_;
    std::vector<Transaction> log_;
public:
    static TransactionManager& getInstance() {
        static TransactionManager instance;
        return instance;
    }

    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;

    void deposit(Account* acc, double amount) {
        if (!acc || amount < 0) return;

        std::scoped_lock lock(acc->mtx_, log_mtx_);

        Transaction tx;
        tx.type = Transaction::Type::Deposit;
        tx.account_id = acc->id_;
        tx.amount = amount;
        tx.before = acc->balance_;

        acc->add(amount);

        tx.after = acc->balance_;
        tx.tid = std::this_thread::get_id();
        tx.timestamp = nowMs();
        tx.ok = true;

        log_.push_back(tx);
    }

    void withdraw(Account* acc, double amount) {
        if (!acc || amount < 0) return;

        std::scoped_lock lock(acc->mtx_, log_mtx_);

        Transaction tx;
        tx.type = Transaction::Type::Withdraw;
        tx.account_id = acc->id_;
        tx.amount = amount;
        tx.before = acc->balance_;

        tx.ok = acc->sub(amount);

        tx.after = acc->balance_;
        tx.tid = std::this_thread::get_id();
        tx.timestamp = nowMs();

        log_.push_back(tx);
    }

    std::size_t transactionsCount() const {
        std::lock_guard<std::mutex> lock(log_mtx_);
        return log_.size();
    }
};



int main() {

    auto& manager = TransactionManager::getInstance();
    Account account("A1", 1000);
    vector<thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
        manager.deposit(&account, 100);
        manager.withdraw(&account, 50);
        });
    }
    for (auto& t : threads) t.join();
    cout << "Balance final: "<< account.getBalance() << endl;
    cout << "Direccion manager: " << &TransactionManager::getInstance() << endl;


    return 0;
}