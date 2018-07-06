#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <vector>
#include <unordered_map>
#include <unordered_set>

class Graph{
public:
    Graph();
    size_t search(const std::unordered_map<size_t, float>& q, size_t enterPoint, size_t* visit);
    std::unordered_set<size_t> multiSearch(const std::unordered_map<size_t, float>& q,
        std::vector<size_t> enterPoints,
        size_t k,
        size_t* visit);
    std::unordered_set<size_t> naiveSearch(const std::unordered_map<size_t, float>& q, size_t k);
    void add(size_t id,
        const std::unordered_map<size_t, float>& point,
        size_t k,
        size_t attempts);
    std::string toString();
    std::tuple<bool, size_t, std::unordered_map<size_t, float> > parseLine(const std::string& line);
    void validate();
private:
    std::vector<size_t> getRandomEnterPoints(size_t m);
    std::unordered_map<size_t, std::unordered_map<size_t, float> > _vertices;
    std::unordered_map<size_t, std::unordered_set<size_t> > _edges;

};

#endif
