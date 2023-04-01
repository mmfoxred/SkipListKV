#include "skiplist.h"
#include <iostream>
using namespace std;

void load_file(SkipList<int, string>& list)
{
    // list.load_file();
    list.display_list();
    cout << "skiplist size: " << list.size() << endl;
}
int main()
{
    SkipList<int, string> list(6);
    list.insert_element(1, "测试list1");
    list.insert_element(2, "测试list3");
    list.insert_element(7, "测试list4");
    list.insert_element(6, "测试list7");
    list.display_list();
    cout << "skiplist size: " << list.size() << endl;
    list.delete_element(2);
    cout << "--------------------------" << endl;
    list.display_list();
    cout << "skiplist size: " << list.size() << endl;
    cout << "--------------------------" << endl;
    list.search_element(2);
    list.search_element(7);
    list.dump_file();
    return 0;
}