#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

int main()
{
    vector<int> v;
    v.push_back(4);
    v.push_back(6);
    v.push_back(8);
    v.push_back(7);
    v.push_back(5);
    v.push_back(3);
    v.push_back(1);
    for(auto e: v) {
        cout << e << " ";
    } cout << endl;
    sort(v.begin(), v.end(), [](const int& lhs, const int& rhs){return lhs>rhs;});
    for(auto e: v) {
        cout << e << " ";
    } cout << endl;
    return 0;
}
