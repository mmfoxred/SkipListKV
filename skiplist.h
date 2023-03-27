#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

template<typename K, typename V>
class Node
{
public:
    Node() = default;
    Node(K k, V v, int level);
    ~Node();
    K    get_key() const;
    V    get_value() const;
    void set_value(V v);
    // level
    Node<K, V>** forward;
    int          node_level;

private:
    K key;
    V value;
};

// 构造
template<typename K, typename V>
Node<K, V>::Node(K k, V v, int level)
    : key(k)
    , value(v)
    , node_level(level)
{
    forward = new Node<K, V>*[level + 1];   //?为什么需要level+1层
    memset(forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

// 析构
template<typename K, typename V>
Node<K, V>::~Node()
{
    delete[] forward;
}

template<typename K, typename V>
K Node<K, V>::get_key() const
{
    return this->key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const
{
    return this->value;
}

template<typename K, typename V>
void Node<K, V>::set_value(V v)
{
    this->value = v;
}

// SkipList
template<typename K, typename V>
class SkipList
{
public:
    SkipList(int level);
    ~SkipList();

    Node<K, V>* createNode(K key, V val, int level);
    int         insert_element(K key, V val);
    void        display_list();
    void        delete_element(K key);
    void        search_element(K key);

    // 持久化
    void load_file();
    void dump_file();

    int get_randLevel();
    int size();

private:
    bool is_str_valid(const std::string& str);
    void get_kV_from_string(const std::string& str, std::string& key, std::string& val);

private:
    int _max_level;         // maximum level of skiplist
    int _skip_list_level;   // current level of skiplist
    int _length;

    Node<K, V>* _header;

    // file
    std::ofstream _file_writer;
    std::ifstream _file_reader;
};
