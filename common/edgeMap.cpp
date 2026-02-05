#include "edgeMap.hpp"

// Build once

void buildEdgeAdjacency(
    const std::vector<uint32_t>& indices,
    EdgeToTriangles& edgeToTris)
{
    edgeToTris.clear();
    edgeToTris.reserve(indices.size() / 2);

    auto addEdge = [&](uint32_t a, uint32_t b, uint32_t triId) {
        edgeToTris[EdgeKey(a,b)].push_back(triId);
    };

    const uint32_t numTris = indices.size() / 3;
    for (uint32_t t = 0; t < numTris; ++t) {
        uint32_t a = indices[3*t + 0];
        uint32_t b = indices[3*t + 1];
        uint32_t c = indices[3*t + 2];

        addEdge(a,b,t);
        addEdge(b,c,t);
        addEdge(c,a,t);
    }
}

int trisSharingABNotC(
    const std::vector<uint32_t>& indices,
    uint32_t a, uint32_t b, uint32_t c, EdgeToTriangles& edgeToTris)
{
  auto it = edgeToTris.find(EdgeKey(a,b));
  for (uint32_t triId : it->second) {
    uint32_t i0 = indices[3*triId+0];
    uint32_t i1 = indices[3*triId+1];
    uint32_t i2 = indices[3*triId+2];

    if (i0 != c && i1 != c && i2 != c)
      return triId;
  }
  return -1;
}
