#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <stack>
#include <memory>

class Graph;

class GraphIterator {
public:
    virtual ~GraphIterator() = default;
    virtual bool hasNext() = 0;
    virtual int next() = 0;
};

class BFSIterator;
class DFSIterator;

class Node {
    int id_;
public:
    explicit Node(int id = 0) : id_(id) {}
    int id() const { return id_; }
};

class Graph {
    friend class BFSIterator;
    friend class DFSIterator;

    std::unordered_map<int, std::vector<int>> adj_;
    std::unordered_map<int, Node> nodes_;

    void ensureNode_(int id) {
        if (nodes_.find(id) == nodes_.end()) nodes_.emplace(id, Node{id});
        if (adj_.find(id) == adj_.end()) adj_.emplace(id, std::vector<int>{});
    }

public:
    void addEdge(int from, int to) {
        ensureNode_(from);
        ensureNode_(to);
        adj_[from].push_back(to);
    }
};

class BFSIterator : public GraphIterator {
    Graph& g_;
    std::queue<int> q_;
    std::unordered_set<int> discovered_;
    bool buffered_ = false;
    int value_ = 0;

    void prepare_() {
        if (buffered_) return;
        if (q_.empty()) return;

        int v = q_.front();
        q_.pop();

        auto it = g_.adj_.find(v);
        if (it != g_.adj_.end()) {
            for (int nei : it->second) {
                if (discovered_.insert(nei).second) q_.push(nei);
            }
        }

        value_ = v;
        buffered_ = true;
    }

public:
    BFSIterator(Graph& g, int start) : g_(g) {
        g_.ensureNode_(start);
        discovered_.insert(start);
        q_.push(start);
    }

    bool hasNext() override {
        prepare_();
        return buffered_;
    }

    int next() override {
        prepare_();
        if (!buffered_) throw std::out_of_range("Iterator exhausted");
        buffered_ = false;
        return value_;
    }
};

class DFSIterator : public GraphIterator {
    Graph& g_;
    std::stack<int> st_;
    std::unordered_set<int> discovered_;
    bool buffered_ = false;
    int value_ = 0;

    void prepare_() {
        if (buffered_) return;
        if (st_.empty()) return;

        int v = st_.top();
        st_.pop();

        auto it = g_.adj_.find(v);
        if (it != g_.adj_.end()) {
            const auto& neigh = it->second;
            for (auto rit = neigh.rbegin(); rit != neigh.rend(); ++rit) {
                int nei = *rit;
                if (discovered_.insert(nei).second) st_.push(nei);
            }
        }

        value_ = v;
        buffered_ = true;
    }

public:
    DFSIterator(Graph& g, int start) : g_(g) {
        g_.ensureNode_(start);
        discovered_.insert(start);
        st_.push(start);
    }

    bool hasNext() override {
        prepare_();
        return buffered_;
    }

    int next() override {
        prepare_();
        if (!buffered_) throw std::out_of_range("Iterator exhausted");
        buffered_ = false;
        return value_;
    }
};

int main() {
    Graph g;
    g.addEdge(1,2);
    g.addEdge(1,3);
    g.addEdge(2,4);

    std::unique_ptr<GraphIterator> it = std::make_unique<BFSIterator>(g, 1);
    while(it->hasNext()) std::cout << it->next() << " ";
    std::cout << "\n";

    it = std::make_unique<DFSIterator>(g, 1);
    while(it->hasNext()) std::cout << it->next() << " ";
    std::cout << "\n";
}