# Grease Pencil OpenGL App for plain objects

Implementing a Greese Pencil functionality for drawing 2D shapes over 3D meshes.

This is for a downstream project + a nice learning experience.

Current State:
1. Working Tri-Mesh -> BVH builder (CPU implementation for triangle picking)
2. On Mouse-Click:
    - Ray-BVHnode traversal for grouping intersecting triangles
    - Ray-Triangle intersection for picking triangles to connect
    - Debugging visualization for connected sequences of surface points 


Current Work:
- A CPU-BVH implementation with a depth-ordered equal split strategy
- A Ray-AABB debugging visualization for ray-BVHnode intersection
- A Ray-AABB traversal with the SLAB method to collect intersected BVHnodes
- Mouse-click to pick a surface position
