#include <set>
#include <iostream>
int main()

{
    std::set<size_t> s;
    s.insert(1);
    s.insert(2);
    s.insert(6);
    s.insert(8);
    s.insert(4);
    s.insert(99);
    s.insert(0);
    s.insert(77);
    s.insert(3);
    s.insert(2);
    s.insert(5);
    for(auto e: s) {
        std::cout << e << std::endl;
    }
    return 0;
}
