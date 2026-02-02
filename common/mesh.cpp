#include "mesh.hpp"
#include "texture.hpp"

#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

Assimp::Importer importer;

BaseMesh::BaseMesh(){
    // TODO constructor
}

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


void BVHMesh::simpleRender(){
    debugLines.reserve(nodes.size() * 24); // 24 verts per box (12 edges)
    for(const auto n: nodes){
        appendAABBWireframe(debugLines, n.box.bmin, n.box.bmax);
    }
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

    if (nodes[NodeIdx].ChildIndex == 0){
        return;
    }
    appendAABBWireframe(debugLines, nodes[NodeIdx].box.bmin, nodes[NodeIdx].box.bmax);

    recursiveHitProgram(nodes[NodeIdx].ChildIndex, ro, rd);
    recursiveHitProgram(nodes[NodeIdx].ChildIndex+1, ro, rd);
}


bool BVHMesh::raycast(const glm::vec3& ro, const glm::vec3& rd){
    debugLines.clear();
    
    recursiveHitProgram(0, ro, rd);

    return true;
}