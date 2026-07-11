#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include "grid.h"
#include "search_result.h"

// A tiny hand-rolled JSON writer. We avoid pulling in a full JSON
// library since our response shapes are simple and fixed -- this
// keeps the dependency footprint to just cpp-httplib.

inline std::string cellsToJson(const std::vector<std::pair<int,int>>& cells) {
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < cells.size(); i++) {
        os << "[" << cells[i].first << "," << cells[i].second << "]";
        if (i + 1 < cells.size()) os << ",";
    }
    os << "]";
    return os.str();
}

inline std::string gridToJson(const Grid& grid) {
    std::ostringstream os;
    os << "{\"rows\":" << grid.rows() << ",\"cols\":" << grid.cols() << ",\"cells\":[";
    for (int r = 0; r < grid.rows(); r++) {
        os << "[";
        for (int c = 0; c < grid.cols(); c++) {
            const auto& cell = grid.at(r, c);
            os << "{\"type\":" << static_cast<int>(cell.type)
               << ",\"weight\":" << cell.weight << "}";
            if (c + 1 < grid.cols()) os << ",";
        }
        os << "]";
        if (r + 1 < grid.rows()) os << ",";
    }
    os << "]}";
    return os.str();
}

inline std::string resultToJson(const SearchResult& res) {
    std::ostringstream os;
    os << "{"
       << "\"visitedOrder\":" << cellsToJson(res.visitedOrder) << ","
       << "\"path\":" << cellsToJson(res.path) << ","
       << "\"pathCost\":" << res.pathCost << ","
       << "\"visitedCount\":" << res.visitedOrder.size() << ","
       << "\"microseconds\":" << res.microseconds
       << "}";
    return os.str();
}

inline int extractInt(const std::string& body, const std::string& key, int def) {
    std::string pattern = "\"" + key + "\":";
    auto pos = body.find(pattern);
    if (pos == std::string::npos) return def;
    pos += pattern.size();
    return std::stoi(body.substr(pos));
}

inline std::string extractString(const std::string& body, const std::string& key, const std::string& def) {
    std::string pattern = "\"" + key + "\":\"";
    auto pos = body.find(pattern);
    if (pos == std::string::npos) return def;
    pos += pattern.size();
    auto end = body.find("\"", pos);
    return body.substr(pos, end - pos);
}
