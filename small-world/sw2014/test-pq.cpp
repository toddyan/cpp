#include <queue>
#include <deque>
#include <iostream>
int main()
{
    auto cmp = [](int lhs, int rhs){return lhs>rhs;};
    std::priority_queue<int,std::deque<int>, decltype(cmp)> q(cmp);
    q.push(2);
    q.push(4);
    q.push(6);
    q.push(5);
    q.push(3);
    while(!q.empty()) {
        std::cout << q.top() << " ";
        q.pop();
    }
    std::cout << std::endl;
    return 0;
}
