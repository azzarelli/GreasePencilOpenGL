#ifndef EDGEMAP_HPP
#define EDGEMAP_HPP

#include <cstdint>
#include <algorithm>
#include <vector>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>


struct EdgeKey {
  uint32_t v0, v1;
  EdgeKey(uint32_t a, uint32_t b) {
    v0 = std::min(a,b);
    v1 = std::max(a,b);
  }
  bool operator==(EdgeKey const& o) const { return v0==o.v0 && v1==o.v1; }
};

struct EdgeKeyHash {
    std::size_t operator()(const EdgeKey& e) const noexcept {
        uint64_t key = (uint64_t(e.v0) << 32) | uint64_t(e.v1);
        return std::hash<uint64_t>{}(key);
    }
};

using EdgeToTriangles = std::unordered_map<EdgeKey, std::vector<uint32_t>, EdgeKeyHash>;


void buildEdgeAdjacency(
    const std::vector<uint32_t>& indices,
    EdgeToTriangles& edgeToTris);

int trisSharingABNotC(
    const std::vector<uint32_t>& indices,
    uint32_t a, uint32_t b, uint32_t c, EdgeToTriangles& edgeToTris);

#endif