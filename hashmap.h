#include <iostream>
#include <list>
#include <vector>

#ifndef HASH_TABLE_NEW_HASH_MAP_H
#define HASH_TABLE_NEW_HASH_MAP_H

#endif

template<typename KeyType, typename ValueType, typename Hash = std::hash<KeyType>>
class HashMap {
public:
    using HashPair = std::pair<const KeyType, ValueType>;
    using iterator = typename std::list<HashPair>::iterator;
    using const_iterator = typename std::list<HashPair>::const_iterator;

    explicit HashMap(const Hash &hash = Hash()) : hash(hash) {
        cnt_elements = 0;
    }

    template<typename T>
    HashMap(const T &begin, const T &end, const Hash &hash = Hash()) : hash(hash) {
        cnt_elements = 0;
        auto tmp = begin;
        while (tmp != end) {
            insert({begin->first, begin->second});
            ++tmp;
        }
    }

    explicit HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> &hash_pairs, const Hash &hash = Hash()) : hash(hash) {
        cnt_elements = 0;
        for (const HashPair &hash_pair : hash_pairs) {
            insert(hash_pair);
        }
    }

    HashMap& operator=(const HashMap& other) {
        auto copy = other.hash_pairs;
        std::swap(hash_pairs, copy);
        cnt_elements = other.cnt_elements;
        table = other.table;
        return *this;
    }

    size_t size() const {
        return cnt_elements;
    }

    bool empty() const {
        return cnt_elements == 0;
    }

    Hash hash_function() const {
        return hash;
    }

    void insert(const std::pair<KeyType, ValueType> &hash_pair) {
        insert_pair(hash_pair);
    }

    void erase(const KeyType &key) {
        erase_key(key);
    }

    iterator find(const KeyType &key) {
        size_t pos = find_key_pos(key);
        if (pos == table.size()) return hash_pairs.end();
        return table[pos].it;
    }

    const_iterator find(const KeyType &key) const {
        size_t pos = find_key_pos(key);
        if (pos == table.size()) return hash_pairs.end();
        return table[pos].it;
    }

    ValueType &operator[](const KeyType &key) {
        size_t pos = find_key_pos(key);
        if (pos < table.size()) return table[pos].it->second;
        insert_pair({key, ValueType()});
        return table[find_key_pos(key)].it->second;
    }

    const ValueType &at(const KeyType &key) const {
        size_t pos = find_key_pos(key);
        if (pos == table.size()) throw std::out_of_range("No such key in the table");
        return table[pos].it->second;
    }

    void clear() {
        auto copy = hash_pairs;
        for (const auto &hash_pair: copy) {
            erase_key(hash_pair.first);
        }
        hash_pairs.clear();
    }

    iterator begin() {
        return hash_pairs.begin();
    }

    const_iterator begin() const {
        return hash_pairs.begin();
    }

    iterator end() {
        return hash_pairs.end();
    }

    const_iterator end() const {
        return hash_pairs.end();
    }

private:
    struct Node {
        bool free;
        size_t PSL;
        typename std::list<HashPair>::iterator it;

        Node() : free(true), PSL(0) {

        }

        Node(typename std::list<HashPair>::iterator it) : free(false), PSL(0), it(it) {
        }
    };

    size_t find_key_pos(const KeyType &key) const {
        if (table.empty()) return 0;
        size_t pos = get_pos(key);
        size_t PSL = 0;
        while (true) {
            if (PSL > table[pos].PSL or table[pos].free) return table.size();
            if (table[pos].it->first == key) return pos;
            ++PSL;
            pos = next_pos(pos);
        }
    }

    inline size_t next_pos(size_t pos) const {
        if (pos + 1 == table.size()) return 0;
        return pos + 1;
    }

    iterator insert_and_get_it(const HashPair &hash_pair) {
        hash_pairs.push_front(hash_pair);
        return hash_pairs.begin();
    }

    void erase_key(const KeyType &key) {
        size_t pos = find_key_pos(key);
        if (pos == table.size()) return;

        --cnt_elements;
        table[pos].free = true;
        hash_pairs.erase(table[pos].it);

        while (true) {
            if (table[next_pos(pos)].PSL == 0 or table[next_pos(pos)].free) break;
            table[pos] = table[next_pos(pos)];
            --table[pos].PSL;
            pos = next_pos(pos);
            table[pos].free = true;
        }
    }

    void insert_pair(const HashPair &hash_pair) {
        if (find_key_pos(hash_pair.first) < table.size()) return;

        ++cnt_elements;
        increase_size();

        size_t pos = get_pos(hash_pair.first);
        Node nd(insert_and_get_it(hash_pair));

        while (true) {
            if (table[pos].free) {
                table[pos] = nd;
                break;
            }
            if (nd.PSL > table[pos].PSL) std::swap(nd, table[pos]);
            pos = next_pos(pos);
            ++nd.PSL;
        }
    }

    void increase_size() {
        if (table.empty()) {
            table.resize(INITIAL_SIZE);
            return;
        }

        if (get_load_factor() > LOAD_FACTOR) {
            auto old_pairs = hash_pairs;
            for (const auto &hash_pair: old_pairs) {
                erase_key(hash_pair.first);
            }
            table.resize(table.size() * 2);
            for (const auto &hash_pair: old_pairs) {
                insert_pair(hash_pair);
            }
        }
    }

    double get_load_factor() const {
        if (table.empty()) return 0;
        return static_cast<double>(cnt_elements) / static_cast<double>(table.size());
    }

    size_t get_pos(const KeyType &key) const {
        if (table.empty()) return 0;
        return hash_function()(key) % table.size();
    }

    std::vector<Node> table;
    std::list<HashPair> hash_pairs;
    size_t cnt_elements;
    Hash hash;

    static constexpr size_t INITIAL_SIZE = 2;
    static constexpr double LOAD_FACTOR = 0.8;
};
