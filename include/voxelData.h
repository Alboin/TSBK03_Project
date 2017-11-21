#pragma once

#include <vector>
#include <math.h>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>


#include "glm/glm.hpp"
#include "lookuptable.h"

class VoxelData
{
public:
    
    VoxelData(const unsigned dimx, const unsigned dimy, const unsigned dimz);
    
    void generateData(const unsigned seed = 0);

    void generateTriangles(const float isovalue);

    void getInfo(bool showdata = false, bool printvertices = false) const;

    void draw() const;

private:

    void createTriangle(unsigned e1, unsigned e2, unsigned e3, const unsigned x, const unsigned y, const unsigned z);

    const glm::ivec3 getPosition(const unsigned v, unsigned x, unsigned y, unsigned z) const;

    const unsigned hashFunction(const unsigned x, const unsigned y, const unsigned z) const;

    void calculateNormals();

    void createVBO();
    void createBuffers();

    // Data structures for the voxels.
    const unsigned _dim_x, _dim_y, _dim_z;
    float _isovalue;
    std::vector<std::vector<std::vector<float>>> _data;

    // Create a lookup-table for the triangle generation.
    LookupTable _table;

    // Data structures for the triangles.
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec3> _normals;
    std::vector<glm::ivec3> _indices;

    std::vector<glm::vec3> _VBOarray;

    GLuint VBO, VAO, EBO;
    
};