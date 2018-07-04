#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tuple>
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
    size_t* visit)
{
    std::unordered_set<size_t> localMinimas;
    for (auto enterPoint: enterPoints) {
        localMinimas.insert(search(q, enterPoint, visit));
    }
    return localMinimas;
}

size_t Graph::naiveSearch(const std::unordered_map<size_t, float>& q)
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
    if (friendDist.empty()) return 0; //TODO
    return std::get<0>(friendDist[0]);
}
void Graph::add(size_t id,
    const std::unordered_map<size_t, float>& point,
    size_t k,
    size_t attempts)
{
    size_t visit = 0;
    std::unordered_set<size_t> localMinimas
        = multiSearch(point, getRandomEnterPoints(attempts), &visit);
    std::unordered_set<size_t> condidateFriends;
    condidateFriends.insert(localMinimas.begin(), localMinimas.end());
    for (auto f: localMinimas) {
        condidateFriends.insert(_edges[f].begin(), _edges[f].end());
    }
    std::vector<std::tuple<size_t, double> > friendDist;
    for (auto f: condidateFriends) {
        friendDist.push_back(std::make_tuple(f, cosineSimularity(_vertices[f], point)));
    }
    sort(friendDist.begin(),
         friendDist.end(),
         [](const std::tuple<size_t, double>& lhs, const std::tuple<size_t, double>& rhs){
            return std::get<1>(lhs) < std::get<1>(rhs);
         }); 
    for (size_t i = 0; i < k && i < friendDist.size(); ++i) {
        size_t f = std::get<0>(friendDist[i]);
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
        add(std::get<1>(t), std::get<2>(t), 30, 5);
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
        auto localMinimas = multiSearch(std::get<2>(t), getRandomEnterPoints(10), &visit);
        std::vector<std::tuple<size_t,double> > friendDist;
        for (auto f: localMinimas) {
            friendDist.push_back(std::make_tuple(f, cosineSimularity(_vertices[f],std::get<2>(t))));
        }
        std::sort(friendDist.begin(),
            friendDist.end(),
            [](const std::tuple<size_t,double>& lhs, const std::tuple<size_t, double>& rhs){
                return std::get<1>(lhs) < std::get<1>(rhs);
            });
        if (friendDist.empty()) continue;
        size_t naiveFriend = naiveSearch(std::get<2>(t));
        std::cout << std::get<1>(t) << "\t" 
            << std::get<0>(friendDist[0]) << "\t" << std::get<1>(friendDist[0]) << "\t"
            << naiveFriend << "\t" << cosineSimularity(std::get<2>(t), _vertices[naiveFriend]) << "\t"
            << visit
            << std::endl;
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
