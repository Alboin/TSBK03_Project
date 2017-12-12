#pragma once

#include <vector>
#include <math.h>
#include <iostream>
#include <map>
#include <omp.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "lookuptable.h"
#include "simplexnoise1234.h"

class VoxelData
{
public:
    
    VoxelData(const unsigned dim, const float gridSize, const glm::vec3 gridCenter = glm::vec3(0));
    
    void generateData(const float noiseScale = 0.1);

    void generateTriangles(const float isovalue = 0.5);

    void getInfo(bool showdata = false, bool printvertices = false, bool printnormals = false) const;
    int getNumberOfTriangles() const { return _indices.size(); };

    void draw() const;
    void drawBoundingBox() const;

private:

    void createTriangle(unsigned e1, unsigned e2, unsigned e3, const unsigned x, const unsigned y, const unsigned z);

    const glm::ivec3 getPosition(const unsigned v, unsigned x, unsigned y, unsigned z) const;
    const glm::vec3 getWorldPosition(const unsigned x, const unsigned y, const unsigned z) const;
    
    void createVBO();
    void createBuffers();

    // Data structures for the voxels.
    const unsigned _dim;
    const float _gridSize;
    const glm::vec3 _gridCenter;
    float _isovalue;
    std::vector<std::vector<std::vector<float>>> _data;

    unsigned _idCounter = 0;
    unsigned getVertexId(const glm::vec3 v1, const glm::vec3 v2);

    // Create a lookup-table for the triangle generation.
    LookupTable _table;

    // Data structures for the triangles.
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec3> _normals;
    std::vector<glm::ivec3> _indices;

    std::vector<glm::vec3> _VBOarray;
    GLuint VBO, VAO, EBO;

    // Data structures for the bounding box.
    std::vector<glm::vec3> _boundingBoxVertices;
    std::vector<unsigned> _boundingBoxIndices;
    GLuint VBO_b, VAO_b, EBO_b;
    

    // Lock for writing vertex/normal and indices in parallel
    omp_lock_t writelock;
    omp_lock_t writelockIndices;



    inline int clamp(int n, int lower, int upper) {
        return std::max(lower, std::min(n, upper));
    }
    
};