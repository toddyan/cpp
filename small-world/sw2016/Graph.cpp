#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tuple>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "Graph.h"
#include "util.h"
auto less = [](const std::tuple<size_t,double>& lhs, const std::tuple<size_t, double>& rhs){return std::get<1>(lhs) < std::get<1>(rhs);};
auto greater = [](const std::tuple<size_t,double>& lhs, const std::tuple<size_t, double>& rhs){return std::get<1>(lhs) > std::get<1>(rhs);};
Graph::Graph()
{
    srand(time(NULL));
    _maxLayer = 0;
    _maxNeighbor = 32*2;
    _maxNeighborGroud = 32*3;
}
std::unordered_set<size_t> Graph::search(size_t layer,
    const std::unordered_map<size_t, float>& q,
    const std::unordered_set<size_t> ep,
    size_t k,
    size_t ef/*result capacity*/,
    size_t* visit)
{
    if (layer > _maxLayer) return std::unordered_set<size_t>();
    std::unordered_set<size_t> visited;
    std::priority_queue<std::tuple<size_t,double>,std::vector<std::tuple<size_t,double>>,decltype(greater)> candidates(greater);
    std::priority_queue<std::tuple<size_t,double>,std::vector<std::tuple<size_t,double>>,decltype(less)> result(less);
    for(auto enterPoint: ep) {
        auto dist = cosineSimularity(_vertices[enterPoint], q);
        ++ *visit;
        visited.insert(enterPoint);
        candidates.push(std::make_tuple(enterPoint, dist));
        result.push(std::make_tuple(enterPoint, dist));
    }
    while (result.size() > ef) {result.pop();}
    while (!candidates.empty()) {
        auto c = candidates.top();
        candidates.pop();
        if (result.size() >= ef && std::get<1>(c) > std::get<1>(result.top())) {break;}
        for (auto fId: _edges[layer][std::get<0>(c)]) {
            if (visited.count(fId)>0) {continue;}
            visited.insert(fId);
            auto dist = cosineSimularity(_vertices[fId],q);
            ++ *visit;
            if (result.size() >= ef && dist > std::get<1>(result.top())) {continue;}
            candidates.push(std::make_tuple(fId, dist));
            result.push(std::make_tuple(fId, dist));
            while (result.size() > ef) {result.pop();}
        }
    }
    while (result.size() > k) {result.pop();}
    std::unordered_set<size_t> ret;
    while (!result.empty()) {
        ret.insert(std::get<0>(result.top()));
        result.pop();
    }
    return ret;
}

std::unordered_set<size_t> Graph::knnsearch(const std::unordered_map<size_t,float>& q, size_t k, size_t* visit)
{
    std::unordered_set<size_t> ep;
    ep.insert(_enterPoint);
    for (size_t i = _maxLayer; i >= 1; --i) {
        ep = search(i, q, ep, 1, 1, visit);
    }
    return search(0, q, ep, k, k*2, visit);
}
std::unordered_set<size_t> Graph::naiveSearch(const std::unordered_map<size_t, float>& q, size_t k)
{
    std::vector<std::tuple<size_t, double>> friendDist;
    for (auto v: _vertices) {
        friendDist.push_back(std::make_tuple(v.first, cosineSimularity(v.second, q)));
    }
    std::sort(friendDist.begin(),
        friendDist.end(),
        [](const std::tuple<size_t, double>& lhs, const std::tuple<size_t, double>& rhs){
            return std::get<1>(lhs) < std::get<1>(rhs);
        });
    std::unordered_set<size_t> result;
    for (auto f: friendDist) {
        if (result.size() > k) break;
        result.insert(std::get<0>(f));
    }
    return result;
}
void Graph::add(size_t id,
    const std::unordered_map<size_t, float>& point,
    size_t k,
    size_t ef)
{
    size_t layer = randomLayer();
    size_t visit;
    if (!_vertices.empty()) {
        std::unordered_set<size_t> ep;
        ep.insert(_enterPoint);
        for (size_t i = std::max(_maxLayer, layer); i > layer; --i) {
            ep = search(i, point, ep, 1, 1, &visit);
            assert(ep.size() >= 1);
        }
        for (long i = static_cast<long>(std::min(_maxLayer, layer)); i>=0; --i) {
            auto maxNeighbor = _maxNeighbor;
            if (i==0) {
                maxNeighbor = _maxNeighborGroud;
            }
            auto layerMinima = search(i, point, ep, k, ef, &visit);
            if (layerMinima.empty()) {return;}
            layerMinima = neighborSelectSimple(point, layerMinima, k);
            for (auto fId: layerMinima) {
                _edges[i][fId].insert(id);
                _edges[i][id].insert(fId);
                //shrink
                if (_edges[i][fId].size() > maxNeighbor) {
                    _edges[i][fId] = neighborSelectSimple(_vertices[fId], _edges[i][fId], maxNeighbor);
                }
                if (_edges[i][id].size() > maxNeighbor) {
                    _edges[i][id] = neighborSelectSimple(_vertices[id], _edges[i][id], maxNeighbor);
                }
            }
            size_t bestDist = SIZE_MAX;
            size_t bestFriendId = 0;
            for (auto fId: layerMinima) {
                auto dist = cosineSimularity(_vertices[fId], point);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestFriendId = fId;
                }
            }
            if (bestDist != SIZE_MAX && i != 0) {
                ep.clear();
                ep.insert(bestFriendId);
            }
        }
    }
    if (_vertices.empty() || layer > _maxLayer) {
        _enterPoint = id;
        _maxLayer = layer;
    }
    _vertices[id] = point;
}
std::unordered_set<size_t> Graph::neighborSelectSimple(const std::unordered_map<size_t,float>& base, const std::unordered_set<size_t>& candidates, size_t k)
{
    if (candidates.size() <= k) return candidates;
    std::priority_queue<std::tuple<size_t, double>, std::vector<std::tuple<size_t, double>>, decltype(greater)> friendDist(greater);
    for (auto c: candidates) {
        friendDist.push(std::make_tuple(c, cosineSimularity(_vertices[c], base)));
    }
    std::unordered_set<size_t> ret;
    while (ret.size() < k) {
        ret.insert(std::get<0>(friendDist.top()));
        friendDist.pop();
    }
    return ret;
}
std::string Graph::toString()
{
    std::stringstream buf;
    for (auto v: _vertices) {
        buf << v.first << "\t";
        for (auto dim: v.second) {
            buf << dim.first << ":" << dim.second << "\t";
        }
        buf << "\n";
    }
    for (auto e: _edges) {
        buf << e.first << "|\t";
        for (auto f: e.second) {
            buf << f.first << ":";
            for (auto lf: f.second) {
                buf << lf << "\t";
            }
        }
        buf << "\n";
    }
    return buf.str();
}

std::tuple<bool, size_t, std::unordered_map<size_t, float>> Graph::parseLine(const std::string& line)
{
    std::stringstream buf(line);
    size_t id;
    std::unordered_map<size_t, float> point;
    size_t dim_index;
    double dim_value;
    if (!(buf >> id)) return std::make_tuple(false, 0, std::unordered_map<size_t, float>());
    while(buf >> dim_index >> dim_value) {
        point[dim_index] = static_cast<float>(dim_value);
    }
    return std::make_tuple(true, id, point);
}

std::vector<size_t> Graph::getRandomEnterPoints(size_t m)
{
    std::vector<size_t> vertices;
    for (auto v: _vertices) {
        vertices.push_back(v.first);
    }
    if (_vertices.empty()) return std::vector<size_t>();
    std::vector<size_t> ret;
    for (size_t i = 0; i < m && vertices.size()>0; ++i) {
        size_t r = rand() % vertices.size();
        ret.push_back(vertices[r]);
        vertices[r] = vertices.back();
        vertices.pop_back();
    }
    return ret;
}
size_t Graph::randomLayer()
{
    double r;
    while((r = rand()/static_cast<double>(RAND_MAX)) <= 0); // (0~1], log(0) is undefined
    size_t negLogR;
    return static_cast<size_t>(-log(r));
}
void Graph::validate()
{
    std::ifstream indexFile("./index");
    std::ifstream queryFile("./query");
    std::string line;
    size_t verbose_count = 0;
    while (getline(indexFile, line)) {
        auto t = parseLine(line);
        if (!std::get<0>(t)) {
            std::cerr << "parse line:[" << line << "] failed." << std::endl;
            continue;
        }
        add(std::get<1>(t), std::get<2>(t), 32, 32*3);
        std::string ch = "";
        if(verbose_count % 100 ==0) {ch = ".";}
        if(verbose_count % 1000 ==0) {ch = "|";}
        std::cerr << ch << std::flush;
        ++ verbose_count;
    }
    std::cerr << "\nindexing finished." << std::endl;
    indexFile.close();
    std::cerr << "graph.size:" << _vertices.size() << std::endl;
    for (size_t layer = 0; layer <= _maxLayer; ++layer) {
        std::cerr << "layer" << layer << ":" << _edges[layer].size() << std::endl;
    }
    if (_vertices.size()==0) return;
    while (getline(queryFile, line)) {
        auto t = parseLine(line);
        if (!std::get<0>(t)) {
            std::cerr << "parse line:[" << line << "] failed." << std::endl;
            continue;
        }
        size_t visit = 0;
        auto naiveFriends = naiveSearch(std::get<2>(t), 10);
        auto kFriends = knnsearch(std::get<2>(t), 10, &visit);
        std::cout << std::get<1>(t) << "\t" << visit << "\t";
        for (auto e: kFriends) {
            std::cout << e << ":" << cosineSimularity(_vertices[e], std::get<2>(t)) << "|";
        }
        std::cout << "\t";
        for (auto e: naiveFriends) {
            std::cout << e << ":" << cosineSimularity(_vertices[e], std::get<2>(t)) << "|";
        }
        std::cout << std::endl;
    }
    queryFile.close();
}
#ifdef TEST_GRAPH
int main()
{
    Graph g;
    g.validate();
}

#endif
