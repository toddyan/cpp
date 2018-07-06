#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tuple>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include "Graph.h"
#include "util.h"
Graph::Graph()
{
    srand(time(NULL));
}
size_t Graph::search(const std::unordered_map<size_t, float>& q, size_t enterPoint, size_t* visit)
{
    size_t cur = enterPoint;
    size_t next = cur;
    double dMin = cosineSimularity(q, _vertices[enterPoint]);
    for (auto neighbor: _edges[cur]) {
        if (cosineSimularity(q, _vertices[neighbor]) < dMin) {
            dMin = cosineSimularity(q, _vertices[neighbor]);
            next = neighbor;
        }
        ++ *visit;
    }
    if (next == cur) return cur;
    return search(q, next, visit);
}

std::unordered_set<size_t> Graph::multiSearch(const std::unordered_map<size_t, float>& q,
    std::vector<size_t> enterPoints,
    size_t k,
    size_t* visit)
{
    auto less = [](const std::tuple<size_t, double>& lhs, const std::tuple<size_t, double>& rhs) {
        return std::get<1>(lhs) < std::get<1>(rhs);
    };
    auto greater = [](const std::tuple<size_t, double>& lhs, const std::tuple<size_t, double>& rhs) {
        return std::get<1>(lhs) > std::get<1>(rhs);
    };
    std::unordered_set<size_t> visitedSet;
    // top is largest
    std::priority_queue<std::tuple<size_t, double>,
        std::vector<std::tuple<size_t, double>>,
        decltype(less)> globalMinina(less);
    // top is smallest
    std::priority_queue<std::tuple<size_t, double>,
        std::vector<std::tuple<size_t, double>>,
        decltype(greater)> condidates(greater);
    for (auto enterPoint: enterPoints) {
        std::priority_queue<std::tuple<size_t, double>,
            std::vector<std::tuple<size_t, double>>,
            decltype(less)> curIterMinima(less);
        condidates.push(std::make_tuple(enterPoint,cosineSimularity(_vertices[enterPoint], q)));
        ++ *visit;
        while (!condidates.empty()) {
            auto c = condidates.top(); //closest
            condidates.pop();
            // ignore visited node
            if (visitedSet.count(std::get<0>(c))) continue;
            visitedSet.insert(std::get<0>(c));
            // all candidates are too far
            if (!curIterMinima.empty() && std::get<1>(c) > std::get<1>(curIterMinima.top())) break; 
            visitedSet.insert(std::get<0>(c));
            curIterMinima.push(c);
            if (curIterMinima.size() > k) curIterMinima.pop();
            for (auto fId: _edges[std::get<0>(c)]) {
                if (visitedSet.count(fId)) continue;
                visitedSet.insert(fId);
                double d = cosineSimularity(_vertices[fId], q);
                ++ *visit;
                condidates.push(std::make_tuple(fId, d));
                curIterMinima.push(std::make_tuple(fId, d));
                if (curIterMinima.size() > k) curIterMinima.pop();
            }
        }
        while(!curIterMinima.empty()) {
            auto f = curIterMinima.top();
            curIterMinima.pop();
            globalMinina.push(f);
            if (globalMinina.size() > k) globalMinina.pop();
        }
    }
    std::unordered_set<size_t> result;
    while(!globalMinina.empty()) {
        auto f = globalMinina.top();
        globalMinina.pop();
        result.insert(std::get<0>(f));
    }
    return result;
}

std::unordered_set<size_t> Graph::naiveSearch(const std::unordered_map<size_t, float>& q, size_t k)
{
    std::vector<std::tuple<size_t, double> > friendDist;
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
    size_t attempts)
{
    size_t visit = 0;
    std::unordered_set<size_t> kFriends = multiSearch(point, getRandomEnterPoints(attempts), k, &visit);
    for (auto f: kFriends) {
        _edges[f].insert(id);
        _edges[id].insert(f);
    }
    _vertices[id] = point;
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
        buf << e.first << ":\t";
        for (auto f: e.second) {
            buf << f << "\t";
        }
        buf << "\n";
    }
    return buf.str();
}

std::tuple<bool, size_t, std::unordered_map<size_t, float> > Graph::parseLine(const std::string& line)
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
        add(std::get<1>(t), std::get<2>(t), 32*3, 30);
        if(verbose_count % 1000 ==0) std::cerr << "." << std::flush;
        ++ verbose_count;
    }
    std::cerr << "\nindexing finished." << std::endl;
    indexFile.close();
    std::cerr << "graph.size:" << _vertices.size() << std::endl;
    if (_vertices.size()==0) return;
    while (getline(queryFile, line)) {
        auto t = parseLine(line);
        if (!std::get<0>(t)) {
            std::cerr << "parse line:[" << line << "] failed." << std::endl;
            continue;
        }
        size_t visit = 0;
        auto kFriends = multiSearch(std::get<2>(t), getRandomEnterPoints(30), 10, &visit);
        auto naiveFriends = naiveSearch(std::get<2>(t), 10);
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
