#include "util.h"
#include "math.h"
double cosineSimularity(const std::unordered_map<size_t, float>& v1, const std::unordered_map<size_t, float>& v2)
{
    double len1 = 0, len2 = 0;
    for (auto e: v1) {
        len1 += e.second * e.second;
    }
    for (auto e: v2) {
        len2 += e.second * e.second;
    }
    len1 = sqrt(len1);
    len2 = sqrt(len2);
    double innerProduct = 0;
    for (auto e: v1) {
        auto iter = v2.find(e.first);
        if (iter != v2.end()) {
            innerProduct += e.second * iter->second;
        }
    }
    double simularity = 0;
    if (len1 * len2 > 0.0000001) {
        simularity = innerProduct / (len1 * len2);
    }
    return acos(simularity);
}

#ifdef TEST_UTIL
#include <iostream>

int main()
{
    std::unordered_map<size_t, float> v1, v2;
    v1[0] = 1;
    v1[1] = 2;
    v1[2] = 3;
    v1[3] = 4;

    v2[1] = 3;
    v2[3] = 4;
    v2[4] = 5;
    
    std::cout << cosineSimularity(v1,v2) << std::endl;
}

#endif
