#include "mesh.hpp"
#include "texture.hpp"


#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/gtx/intersect.hpp>

Assimp::Importer importer;

BaseMesh::BaseMesh(){
    // TODO constructor
}

inline bool DEBUG = false;

void BaseMesh::load(std::string filePath, const char* filePathDSS, GLuint programID){
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    Texture = loadDDS(filePathDSS);
    TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    // Read object file
    scene = importer.ReadFile(
        filePath,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices
    );

    // Loading check
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        fprintf(stderr, "Failer to load object");
    }
    if (scene->mNumMeshes == 0) {
        fprintf(stderr, "No meshes found in file\n");
    }

    // Load Assimp properties into our mesh's vertices, uvs, normals and index buffers
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        // remember where this mesh's vertices start in the big arrays
        unsigned int baseVertex = (unsigned int)vertices.size();

        // vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            vertices.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->HasNormals())
                normals.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            else
                normals.emplace_back(0.0f, 0.0f, 1.0f);

            if (mesh->HasTextureCoords(0))
                uvs.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                uvs.emplace_back(0.0f, 0.0f);
        }

        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace& face = mesh->mFaces[f];
            if (face.mNumIndices != 3) continue; // should be 3 after triangulate

            indices.push_back(baseVertex + face.mIndices[0]);
            indices.push_back(baseVertex + face.mIndices[1]);
            indices.push_back(baseVertex + face.mIndices[2]);
        }
    }

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER,
                vertices.size() * sizeof(glm::vec3),
                vertices.data(),
                GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER,
                uvs.size() * sizeof(glm::vec2),
                uvs.data(),
                GL_STATIC_DRAW);

    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER,
                normals.size() * sizeof(glm::vec3),
                normals.data(),
                GL_STATIC_DRAW);

    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                indices.size() * sizeof(unsigned int),
                indices.data(),
                GL_STATIC_DRAW);
    printf("Object Info: verts=%zu uvs=%zu normals=%zu indices=%zu\n",
       vertices.size(), uvs.size(), normals.size(), indices.size());


    // Build Edge Map
    buildEdgeAdjacency(indices, edgeToTris);
    size_t boundaryEdges = 0, manifoldEdges = 0, nonManifold = 0;
    for (auto& [e, tris] : edgeToTris) {
        if (tris.size() == 1) boundaryEdges++;
        else if (tris.size() == 2) manifoldEdges++;
        else nonManifold++;
    }
    printf("boundary=%zu manifold=%zu nonmanifold=%zu\n", boundaryEdges, manifoldEdges, nonManifold);

}


void BaseMesh::step(){
    glBindVertexArray(VertexArrayID);   // <<< REQUIRED in core profile
    
    // First bind texture to a uni
    glActiveTexture(GL_TEXTURE0); // Get active texture unit 0
    glBindTexture(GL_TEXTURE_2D, Texture); // Bind the current texture to a 2DGL tex
    glUniform1i(TextureID, 0); // Sample from texture unit 0


    // Configuration for the vertex positions/buffer
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // Configuration for the UV buffer
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );


    // Draw the triangle !
    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(
        GL_TRIANGLES,      // mode
        indices.size(),    // count
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void BaseMesh::del(){
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

}


AABB::AABB(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    centroid = (a + b + c) * (1.0f / 3.0f);

    bmin = glm::vec3(
        std::min({a.x, b.x, c.x}),
        std::min({a.y, b.y, c.y}),
        std::min({a.z, b.z, c.z})
    );

    bmax = glm::vec3(
        std::max({a.x, b.x, c.x}),
        std::max({a.y, b.y, c.y}),
        std::max({a.z, b.z, c.z})
    );
}

AABB::AABB(const std::vector<glm::vec3>& points) {
    centroid = glm::vec3(0.0f);
    bmin = bmax = points[0];

    for (const auto& p: points){
        bmin = glm::min(bmin, p);
        bmax = glm::max(bmax, p);
        centroid += p;
    }

    centroid /= static_cast<float>(points.size());
}

AABB::AABB(const std::vector<glm::vec3>& verts, std::vector<unsigned int> indices) {
    centroid = glm::vec3(0.0f);
    bmin = bmax = verts[indices[0]];

    for (unsigned int idx : indices)
    {
        const glm::vec3& v = verts[idx];
        bmin = glm::min(bmin, v);
        bmax = glm::max(bmax, v);
        centroid += v;
    }
    centroid /= static_cast<float>(indices.size());
}

int AABB::longest_axis(){
    glm::vec3 blen = bmax - bmin;
    int axis = 0;
    if(blen.y > blen.x) axis=1;
    if(blen.z > blen[axis]) axis = 2;
    return axis;
}

void BVHMesh::splitNode(uint32_t ParentIdx){

    const std::vector<unsigned int>& index_pass =  nodes[ParentIdx].indices;

    uint32_t triCount = (uint32_t) (index_pass.size() / 3);

    // Deal with Child
    if(triCount <= leafMaxSize){
        // Handle Child/Leaf
        return;
    }

    // Get the axis for splitting lists
    int axis = nodes[ParentIdx].box.longest_axis();

    // If the axis is small it becomes a child
    if((nodes[ParentIdx].box.bmax[axis] - nodes[ParentIdx].box.bmin[axis]) < 1e-8){
        // Handle Child/Leaf
        return;
    }
    
    // Checks passed! This is a parent node

    // For now take the longest axis of the AABB
    // Go through each triangle
    std::vector<uint32_t> triRefs(triCount);
    for(uint32_t i = 0; i < triCount; i++){
        triRefs[i] = i;
    }

    // Sort the list of triangle references based on the average vertex position along
    //  a given axis
    std::sort(triRefs.begin(), triRefs.end(),
    [&](uint32_t a, uint32_t b)
    {
        auto centroid = [&](uint32_t i)
        {
            return (
                vertices[index_pass[i*3 + 0]] +
                vertices[index_pass[i*3 + 1]] +
                vertices[index_pass[i*3 + 2]]
            ) * (1.0f / 3.0f);
        };

        return centroid(a)[axis] < centroid(b)[axis];
    });

    // the mid point of the list - even split
    uint32_t mid = triCount / 2;

    // Pass into left and right indices
    std::vector<unsigned int> leftIndices;
    std::vector<unsigned int> rightIndices;
    for (uint32_t i = 0; i < triCount; i++)
    {   
        auto& dst = (i < mid) ? leftIndices : rightIndices;

        dst.push_back(index_pass[triRefs[i]*3 + 0]);
        dst.push_back(index_pass[triRefs[i]*3 + 1]);
        dst.push_back(index_pass[triRefs[i]*3 + 2]);
    }
    

    Node leftNode =  Node{AABB(vertices, leftIndices), 0, leftIndices};
    Node rightNode =  Node{AABB(vertices, rightIndices), 0, rightIndices};
    
    // Update parent node with childNode position
    uint32_t childIndex = (uint32_t)nodes.size();
    nodes[ParentIdx].ChildIndex = childIndex;
    nodes.push_back(leftNode);
    nodes.push_back(rightNode);

    // Process parent node
    splitNode(childIndex);
    splitNode(childIndex+1);

}

void BVHMesh::buildBVH(uint32_t leafSize){
    // Set the max number of triangles per box
    leafMaxSize = std::max<uint32_t>(1, leafSize);
    
    // Make sure we are dealing with triangles
    if(indices.size() % 3 != 0){
        fprintf(stderr, "BVHMesh:buildBVH - Cant resolve triangles from given indices");
       return; 
    }

    uint32_t triCount = (uint32_t)(indices.size() / 3);
    
    nodes.clear();
    
    Node parentNode = Node{AABB(vertices), 0, indices};
    nodes.push_back(parentNode);

    splitNode(0);
    
    debugLines.reserve(nodes.size() * 2);

}


static inline void appendAABBWireframe(
    std::vector<glm::vec3>& out,
    const glm::vec3& bmin,
    const glm::vec3& bmax
){
    // 8 corners
    glm::vec3 c000(bmin.x, bmin.y, bmin.z);
    glm::vec3 c100(bmax.x, bmin.y, bmin.z);
    glm::vec3 c010(bmin.x, bmax.y, bmin.z);
    glm::vec3 c110(bmax.x, bmax.y, bmin.z);

    glm::vec3 c001(bmin.x, bmin.y, bmax.z);
    glm::vec3 c101(bmax.x, bmin.y, bmax.z);
    glm::vec3 c011(bmin.x, bmax.y, bmax.z);
    glm::vec3 c111(bmax.x, bmax.y, bmax.z);

    auto addEdge = [&](const glm::vec3& a, const glm::vec3& b){
        out.push_back(a);
        out.push_back(b);
    };

    // bottom rectangle (z = bmin.z)
    addEdge(c000, c100);
    addEdge(c100, c110);
    addEdge(c110, c010);
    addEdge(c010, c000);

    // top rectangle (z = bmax.z)
    addEdge(c001, c101);
    addEdge(c101, c111);
    addEdge(c111, c011);
    addEdge(c011, c001);

    // vertical edges
    addEdge(c000, c001);
    addEdge(c100, c101);
    addEdge(c110, c111);
    addEdge(c010, c011);
}

static inline void appendEdge(
    std::vector<glm::vec3>& out,
    const glm::vec3& a,
    const glm::vec3& b
){
    out.push_back(a);
    out.push_back(b);
}

// Ray-AABB via Slab method : https://tavianator.com/2022/ray_box_boundary.html
bool rayAABB_slab(const glm::vec3& ro, const glm::vec3& rd, const AABB& aabb){
    glm::vec3 invD = 1.0f / rd;  // beware zeros; see below
    glm::vec3 t0s = (aabb.bmin - ro) * invD;
    glm::vec3 t1s = (aabb.bmax - ro) * invD;

    glm::vec3 tsmaller = glm::min(t0s, t1s);
    glm::vec3 tbigger  = glm::max(t0s, t1s);

    float tmin = std::max(std::max(tsmaller.x, tsmaller.y), tsmaller.z);
    float tmax = std::min(std::min(tbigger.x,  tbigger.y),  tbigger.z);

    return tmax >= std::max(tmin, 0.0f);

}

void BVHMesh::recursiveHitProgram(uint32_t NodeIdx, const glm::vec3& ro, const glm::vec3& rd){
    bool hit = rayAABB_slab(ro, rd, nodes[NodeIdx].box);

    if(hit==false){
        return;
    }

    // if a child is hit
    if (nodes[NodeIdx].ChildIndex == 0){
        tempNodeBuffer.push_back(NodeIdx);
        return;
    }

    recursiveHitProgram(nodes[NodeIdx].ChildIndex, ro, rd);
    recursiveHitProgram(nodes[NodeIdx].ChildIndex+1, ro, rd);
}

bool BVHMesh::rayNodeIntersection(uint32_t NodeIdx, const glm::vec3& ro, const glm::vec3& rd){
    // For each triangle in the node

    uint32_t triCount = nodes[NodeIdx].indices.size() /3;
    for(uint32_t i=0; i<triCount; i++){
        // Read-only and no copy
        const glm::vec3& v0 = vertices[nodes[NodeIdx].indices[i*3]];
        const glm::vec3& v1 = vertices[nodes[NodeIdx].indices[i*3+1]];
        const glm::vec3& v2 = vertices[nodes[NodeIdx].indices[i*3+2]];
        
        glm::vec2 bary;
        float t;
        bool hit = glm::intersectRayTriangle(ro, rd, v0, v1, v2, bary, t);
        if (hit){
            glm::vec3 hitPoint = ro+(t)*rd;
            debugLines.push_back(hitPoint);

            std::array<unsigned int, 3> dIndList = {nodes[NodeIdx].indices[i*3], nodes[NodeIdx].indices[i*3+1], nodes[NodeIdx].indices[i*3+2]};
            appendAABBWireframe(debugLines, nodes[NodeIdx].box.bmin, nodes[NodeIdx].box.bmax);

            drawTriIndices.push_back(dIndList);
            return true;

        }

    }
    return false;    
}

bool BVHMesh::rayNodeClosestHit(uint32_t NodeIdx,const glm::vec3& ro,const glm::vec3& rd,float& outT,uint32_t& outTriBase ){
    bool anyHit = false;
    float bestT = 1e30f;
    uint32_t bestBase = 0;

    uint32_t triCount = (uint32_t)(nodes[NodeIdx].indices.size() / 3);
    for (uint32_t i = 0; i < triCount; i++) {
        const glm::vec3& v0 = vertices[nodes[NodeIdx].indices[i*3 + 0]];
        const glm::vec3& v1 = vertices[nodes[NodeIdx].indices[i*3 + 1]];
        const glm::vec3& v2 = vertices[nodes[NodeIdx].indices[i*3 + 2]];

        glm::vec2 bary;
        float t;
        bool hit = glm::intersectRayTriangle(ro, rd, v0, v1, v2, bary, t);

        // IMPORTANT: only accept hits in front of the ray origin
        if (hit && t > 0.0f && t < bestT) {
            bestT = t;
            bestBase = i * 3;
            anyHit = true;
        }
    }

    if (!anyHit) return false;

    outT = bestT;
    outTriBase = bestBase;
    return true;
}



float epsT = 1e-6;
void BVHMesh::recursiveTriangleTraversal(glm::vec3 A, glm::vec3 B, std::array<unsigned int, 3> triIndices,std::array<unsigned int, 3> triIndicesB, int depth ){
    // Avoid infinite loops
    if (depth > 1024) { printf("Traversal depth exceeded\n"); return; }
    
    // The B-exit condition, i.e. we are on the triangle where our endpoint lies
    if (triIndices[0] == triIndicesB[0] && triIndices[1] == triIndicesB[1] && triIndices[2] == triIndicesB[2]){
        debugLines.push_back(B);
        return;
    }

    // // 
    // auto maxV = vertices.size();
    // if (triIndices[0] >= maxV || triIndices[1] >= maxV || triIndices[2] >= maxV) {
    //     printf("Bad triIndices: %u %u %u (verts=%zu)\n",
    //         triIndices[0], triIndices[1], triIndices[2], maxV);
    //     return;
    // }

   
    // Get current triangle vertices
    unsigned int i0 = triIndices[0];
    unsigned int i1 = triIndices[1];
    unsigned int i2 = triIndices[2];
    const glm::vec3& v0 = vertices[i0];
    const glm::vec3& v1 = vertices[i1];
    const glm::vec3& v2 = vertices[i2];
    
    glm::vec3 n3 = glm::cross(v1 - v0, v2 - v0); // Triangle plane normal
    float nLen2 = glm::dot(n3,n3); 
    glm::vec3 n = n3 / glm::sqrt(nLen2); // Unit length of normal

    // Function for projecting A and B onto the triangle plane
    auto projectToPlane = [&](const glm::vec3& P){
        float d = glm::dot(n, P - v0); // Think of this as getting the rotational transform of the line P-v0 to transfrom P
        return P - n * d;
    };
    glm::vec3 Ap = projectToPlane(A);
    glm::vec3 Bp = projectToPlane(B);


    glm::vec3 e01 = v1 - v0;
    glm::vec3 uAxis = glm::normalize(e01); // define basis axis for transformation in triangle-space
    glm::vec3 vAxis = glm::normalize(glm::cross(n, uAxis));
    auto to2D = [&](const glm::vec3& P){
        glm::vec3 x = P - v0;
        return glm::vec2(glm::dot(x,uAxis), glm::dot(x,vAxis)); // projection to triangle space
    };

    // Deal with the seg-seg intersection in the triangles coordinate space (2D)
    glm::vec2 a = to2D(Ap);
    glm::vec2 b = to2D(Bp);
    glm::vec2 t0 = to2D(v0), t1 = to2D(v1), t2 = to2D(v2);

    auto cross2 = [](const glm::vec2& p, const glm::vec2& q){ // we need a 2D cross product function
        return p.x*q.y - p.y*q.x;
    };
    auto segSegIntersect2D = [&](
        const glm::vec2& p, const glm::vec2& p2,
        const glm::vec2& q, const glm::vec2& q2,
        float& tOut) -> bool
    {
        glm::vec2 r = p2 - p;
        glm::vec2 s = q2 - q;
        float den = cross2(r, s);
        if (fabs(den) < 1e-12f) return false;

        // Check if the intersection along the line occurs between 0&1 for t and u
        glm::vec2 qp = q - p;
        float t = cross2(qp, s) / den;
        float u = cross2(qp, r) / den;
        if (t >= 0.f && t <= 1.f && u >= 0.f && u <= 1.f) {
            tOut = t;
            return true;
        }
        return false;
    };

    // Find first edge we hit leaving the triangle
    float bestT = 1e30f;
    int hitEdge = -1;


    float tHit;
    if (segSegIntersect2D(a, b, t0, t1, tHit) && tHit > epsT && tHit < bestT) { bestT = tHit; hitEdge = 0; }
    if (segSegIntersect2D(a, b, t1, t2, tHit) && tHit > epsT && tHit < bestT) { bestT = tHit; hitEdge = 1; }
    if (segSegIntersect2D(a, b, t2, t0, tHit) && tHit > epsT && tHit < bestT) { bestT = tHit; hitEdge = 2; }

    if (hitEdge == -1) {
        // Usually means numerical issues, or A->B never exits in 2D due to eps.
        return;
    }

    // World-space intersection point (on the plane)
    glm::vec3 I = Ap + (Bp - Ap) * bestT;

    // Determine which mesh edge we crossed and hop
    uint32_t ea, eb, ec;
    unsigned int eOrd;
    if (hitEdge == 0) { ea = i0; eb = i1; ec=i2;}
    else if (hitEdge == 1) { ea = i1; eb = i2;ec=i0;}
    else { eb = i2; ea = i0; ec=i1;}

    // Find the triangle that shared the edge with this triangle
    int nextTriIndices = trisSharingABNotC(indices, ea, eb, ec, edgeToTris);


    glm::vec3 dir3 = glm::normalize(B - A);
    I = projectToPlane(I);

    debugLines.push_back(I);

    // Check if the next triangle was found // If not end here 
    if (nextTriIndices < 0) {
        printf("Variable `nextTriIndices` not found \n");
        return;
    }
    std::array<unsigned int, 3> NewIndice = { // New set of indices
        indices[nextTriIndices*3 + 0], 
        indices[nextTriIndices*3 + 1],
        indices[nextTriIndices*3 + 2]
    };

    // Nudge a tiny distance so we are safely inside the next tri (tune eps)
    const glm::vec3& Ea = vertices[ea];
    const glm::vec3& Eb = vertices[eb];
    const glm::vec3& Ec = vertices[ec];

    // in-plane "outward" direction perpendicular to edge
    glm::vec3 edgeDir = glm::normalize(Eb - Ea);
    glm::vec3 outDir  = glm::normalize(glm::cross(n, edgeDir));

    // choose sign so it points away from the opposite vertex (ec)
    if (glm::dot(outDir, Ec - Ea) > 0.0f) outDir = -outDir;

    // now nudge slightly outside the current tri, i.e. into the neighbor
    float epsPush = 1e-4f; // tune to your mesh scale
    glm::vec3 Ipush = I + outDir * epsPush;
    recursiveTriangleTraversal(Ipush, B, NewIndice, triIndicesB, depth+1);
}


void BVHMesh::surfaceTraversal(){
    glm::vec3 A = surfaceNodes.at(surfaceNodes.size() - 2); // old vertex
    glm::vec3 B = surfaceNodes.at(surfaceNodes.size() - 1); // newly added point
    std::array<unsigned int, 3> triIndicesA = drawTriIndices.at(drawTriIndices.size() - 2);
    std::array<unsigned int, 3> triIndicesB = drawTriIndices.at(drawTriIndices.size() - 1);
    recursiveTriangleTraversal(A, B, triIndicesA, triIndicesB, 0);
}

float x = 0.0f; // NDC x in [-1,1]
float y = 0.0f; // NDC y in [-1,1]
void BVHMesh::loadNewDrawPoint(glm::mat4 InvMVP, glm::mat4 InvModelMatrix){
    glm::vec4 nearH = InvMVP * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 farH  = InvMVP * glm::vec4(x, y,  1.0f, 1.0f);

    glm::vec3 nearP = glm::vec3(nearH) / nearH.w;
    glm::vec3 farP  = glm::vec3(farH)  / farH.w;

    glm::vec3 rayDir_world = glm::normalize(farP - nearP);
    glm::vec3 rayOrig_world = nearP;
    glm::vec3 rayOrig_model = glm::vec3(InvModelMatrix * glm::vec4(rayOrig_world, 1.0f));
    glm::vec3 rayDir_model  = glm::normalize(glm::vec3(InvModelMatrix * glm::vec4(rayDir_world, 0.0f)));
    
    tempNodeBuffer.clear();
    recursiveHitProgram(0, rayOrig_model, rayDir_model);

    // Nothing to add
    if(tempNodeBuffer.size() < 1){
        return;
    }

    float bestT = 1e30f;
    uint32_t bestNode = 0;
    uint32_t bestTriBase = 0;
    bool anyHit = false;

    for (uint32_t nidx : tempNodeBuffer) {
        float t;
        uint32_t triBase;
        if (rayNodeClosestHit(nidx, rayOrig_model, rayDir_model, t, triBase)) {
            if (t < bestT) {
                bestT = t;
                bestNode = nidx;
                bestTriBase = triBase;
                anyHit = true;
            }
        }
    }

    if (!anyHit) {
        printf("No triangle hit in candidate leaves\n");
        return;
    }

    // optional: draw the leaf AABB
    // appendAABBWireframe(debugLines, nodes[bestNode].box.bmin, nodes[bestNode].box.bmax);

    // Store 3D hit point in model space
    glm::vec3 hitPoint = rayOrig_model + bestT * rayDir_model;
    surfaceNodes.push_back(hitPoint);
    
    // store triangle indices (the actual vertex indices)
    std::array<unsigned int, 3> dIndList = {
        nodes[bestNode].indices[bestTriBase + 0],
        nodes[bestNode].indices[bestTriBase + 1],
        nodes[bestNode].indices[bestTriBase + 2]
    };
    drawTriIndices.push_back(dIndList);

    if(surfaceNodes.size() > 1){
        surfaceTraversal();
    }

}