#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <vector>
#include <unordered_map>
#include <unordered_set>

class Graph{
public:
    Graph();
    std::unordered_set<size_t> search(size_t layer,
        const std::unordered_map<size_t, float>& q,
        const std::unordered_set<size_t> ep,
        size_t k,
        size_t ef/*result capacity*/,
        size_t* visit);
    std::unordered_set<size_t> knnsearch(const std::unordered_map<size_t,float>& q, size_t k, size_t* visit);
    std::unordered_set<size_t> naiveSearch(const std::unordered_map<size_t, float>& q, size_t k);
    void add(size_t id,
        const std::unordered_map<size_t, float>& point,
        size_t k,
        size_t ef);
    std::string toString();
    std::tuple<bool, size_t, std::unordered_map<size_t, float>> parseLine(const std::string& line);
    void validate();
private:
    std::vector<size_t> getRandomEnterPoints(size_t m);
    size_t randomLayer();
    std::unordered_set<size_t> neighborSelectSimple(const std::unordered_map<size_t,float>& base, const std::unordered_set<size_t>& candidates, size_t k);
    std::unordered_map<size_t, std::unordered_map<size_t, float>> _vertices;
    std::unordered_map<size_t/*layer*/, std::unordered_map<size_t/*vertex id*/, std::unordered_set<size_t>>> _edges;
    size_t _maxLayer;
    size_t _enterPoint;
    size_t _maxNeighbor;
    size_t _maxNeighborGroud;
};

#endif
