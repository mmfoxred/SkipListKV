#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string>

std::mutex               mtx;
const static char        delimit = ':';
const static std::string STORE_FILE("./store/dumpfile");

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
    Node<K, V>** forward;   // a array to next node
    int          get_node_level();

private:
    K   key;
    V   value;
    int node_level;   // max level of forward in this node
};

// 构造
template<typename K, typename V>
Node<K, V>::Node(K k, V v, int level)
    : key(k)
    , value(v)
    , node_level(level)
{
    this->forward = new Node<K, V>*[level + 1];   //?为什么需要level+1层
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

// 析构
template<typename K, typename V>
Node<K, V>::~Node()
{
    delete[] forward;
}

template<typename K, typename V>
inline K Node<K, V>::get_key() const
{
    return this->key;
}

template<typename K, typename V>
inline V Node<K, V>::get_value() const
{
    return this->value;
}

template<typename K, typename V>
inline void Node<K, V>::set_value(V v)
{
    this->value = v;
}

template<typename K, typename V>
inline int Node<K, V>::get_node_level()
{
    return this->node_level;
}

// SkipList
template<typename K, typename V>
class SkipList
{
public:
    SkipList(int level);
    ~SkipList();

    Node<K, V>* createNode(const K key, const V val, int level);
    int         insert_element(K key, V val);
    void        display_list();
    void        delete_element(K key);
    bool        search_element(K key);

    // 持久化
    void load_file();
    void dump_file();

    int get_randLevel();
    int size();

private:
    bool is_str_valid(const std::string& str);
    void get_kV_from_string(const std::string& str, std::string& key, std::string& val);
    void destroy_skiplist(Node<K, V>* node);

private:
    int _max_level;         // maximum level of skiplist
    int _skip_list_level;   // current level of skiplist
    int _length;

    Node<K, V>* _header;

    // file
    std::ofstream _file_writer;
    std::ifstream _file_reader;
};

template<typename K, typename V>
inline SkipList<K, V>::SkipList(int max_level)
    : _max_level(max_level)
    , _skip_list_level(0)
    , _length(0)
{
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, max_level);
}

template<typename K, typename V>
inline SkipList<K, V>::~SkipList()
{
    if (_file_reader.is_open())
    {
        _file_reader.close();
    }
    if (_file_writer.is_open())
    {
        _file_writer.close();
    }
    Node<K, V>* current = this->_header->forward[0];   // 直接来到0层
    while (current)                                    // 删除kv节点
    {
        Node<K, V>* tmp = current->forward[0];
        delete current;
        current = tmp;
    }
    delete this->_header;
}

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::createNode(const K key, const V val, int level)
{
    // Node<K, V>* n = new Node<K, V>(key, val, level);
    return new Node<K, V>(key, val, level);
}

template<typename K, typename V>
int SkipList<K, V>::insert_element(K key, V val)
{
    mtx.lock();
    // prepare para for finding all node needed to update
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * _max_level + 1);
    Node<K, V>* current = this->_header;
    // start with the highest level
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }
    // to the lowest level
    current = current->forward[0];
    // if this key has existed
    if (current != nullptr && current->get_key() == key)
    {
        std::cout << "key:" << key << " existed" << std::endl;
        mtx.unlock();
        return -1;   // -1表示已存在
    }
    // if this key doesn't existed
    else if (current == nullptr || current->get_key() != key)
    {
        int randLevel = get_randLevel();
        if (randLevel > _skip_list_level)   // 注意这里是_skip_list_level的值
        {
            for (int i = _skip_list_level + 1; i < randLevel + 1; i++)
                update[i] = _header;
            this->_skip_list_level = randLevel;
        }
        Node<K, V>* node = createNode(key, val, randLevel);
        // insert node
        // 这里要使用randLevel，并不是randLevel ==
        //_skip_list_level，而是在randLevel>_skip_list_level才会有这样的结果,一般还是要取randLevel
        // 为什么下面出错了，定位到上面分配内存的地方？
        for (int i = 0; i <= randLevel; i++)
        {
            node->forward[i]      = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
        std::cout << "successfully inserted key: " << key << ",value: " << val << std::endl;
        ++this->_length;
    }
    mtx.unlock();
    return 0;
}

template<typename K, typename V>
inline void SkipList<K, V>::display_list()
{
    // display this skiplist from high to low
    std::cout << "Display this Skiplist" << std::endl;
    std::cout << "skip_list_level: " << _skip_list_level << std::endl;
    for (int i = _skip_list_level; i >= 0; i--)
    {
        Node<K, V>* node = this->_header->forward[i];
        std::cout << "Level: " << i << std::endl;
        while (node != nullptr)
        {
            std::cout.width(4);
            std::cout << node->get_key() << delimit << node->get_value();
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

template<typename K, typename V>
void SkipList<K, V>::delete_element(K key)
{
    mtx.lock();
    Node<K, V>* current = this->_header;
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));
    // 找出所有需要删除的节点
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        update[i] = current;
    }
    // 来到0层处
    current = current->forward[0];
    // 如果这个位置是key，那么删除所有层对应的节点
    if (current != nullptr && current->get_key() == key)
    {
        // 删除节点
        /*
        for (int i = 0; i <= _skip_list_level; i++)
        {
            if (update[i]->forward[i] == current)
                update[i]->forward[i] = current->forward[i];
            else
                break;   // 因为是从低到高层遍历的，如果不是了，那么上层也必然不会存在该节点了，可以直接退出
        }
        */
        // 另一种写法
        int node_level = current->get_node_level();
        for (int i = node_level; i >= 0; i--)
        {
            update[i]->forward[i] = current->forward[i];
        }
        // 处理个数
        while (_skip_list_level >= 0 && _header->forward[_skip_list_level] == nullptr)
        {
            --_skip_list_level;
        }
        delete current;
        std::cout << "Successfully deleted key " << key << std::endl;
        --_length;
    }
    mtx.unlock();
    return;
}

template<typename K, typename V>
bool SkipList<K, V>::search_element(K key)
{
    std::cout << "Search Key..." << std::endl;
    Node<K, V>* current = this->_header;
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
    }
    // 来到第0层处
    current = current->forward[0];
    if (current && current->get_key() == key)
    {
        std::cout << "Found " << key << delimit << current->get_value() << std::endl;
        return true;
    }
    else
    {
        std::cout << "Not Found key:" << key << std::endl;
        return false;
    }
}

template<typename K, typename V>
inline void SkipList<K, V>::load_file()
{
    std::cout << "load file..." << std::endl;
    _file_reader.open(STORE_FILE, std::ios_base::in);
    std::string key;
    std::string val;
    std::string line;
    while (std::getline(_file_reader, line))
    {
        get_kV_from_string(line, key, val);
        if (key.empty() || val.empty())
            continue;
        insert_element(key, val);
        std::cout << key << delimit << val << std::endl;
    }
    _file_reader.close();
}

template<typename K, typename V>
void SkipList<K, V>::dump_file()
{
    std::cout << "dump file..." << std::endl;
    _file_writer.open(STORE_FILE, std::ios_base::out | std::ios_base::trunc);
    Node<K, V>* current = this->_header->forward[0];
    while (current)
    {
        _file_writer << current->get_key() << delimit << current->get_value() << "\n";
        std::cout << current->get_key() << delimit << current->get_value() << std::endl;
        current = current->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return;
}

template<typename K, typename V>
inline int SkipList<K, V>::get_randLevel()
{
    int k = 0;   // 这里要从level == 0开始，不然1层与0层相同
    while (rand() % 2)
    {
        ++k;
    }
    return std::min(k, this->_max_level);
}

template<typename K, typename V>
inline int SkipList<K, V>::size()
{
    return this->_length;
}

template<typename K, typename V>
inline bool SkipList<K, V>::is_str_valid(const std::string& str)
{
    if (str.empty())
        return false;
    if (str.find(delimit) == std::string::npos)
        return false;
    return true;
}

template<typename K, typename V>
inline void SkipList<K, V>::get_kV_from_string(const std::string& str, std::string& key, std::string& val)
{
    if (!is_str_valid(str))
        return;
    auto index = str.find(delimit);
    key        = str.substr(0, index);
    val        = str.substr(index + 1, str.length());
}

template<typename K, typename V>
void SkipList<K, V>::destroy_skiplist(Node<K, V>* node)
{
    if (node == nullptr)
        return;
    destroy_skiplist(node->forward[0]);
    delete node;
}
