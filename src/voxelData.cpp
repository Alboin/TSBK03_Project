#include "voxelData.h"

// Function for printing glm::vec3 for debugging.
std::ostream& operator<<(std::ostream& os, const glm::vec3 &v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
} 

VoxelData::VoxelData(const unsigned dim, const float gridSize, const glm::vec3 gridCenter)
: _dim(dim), _gridSize(gridSize), _gridCenter(gridCenter), _table(LookupTable())
{
    omp_init_lock(&writelock);
    omp_init_lock(&writelockIndices);

    //std::cout << "Allocating memory... ";
    // Resize data-holder to the given dimensions and initiate it with 0's.
    _data.resize(dim + 1);
    for(unsigned i = 0; i < dim + 1; i++)
    {
        _data[i].resize(dim + 1);
        for(unsigned j = 0; j < dim + 1; j++)
        {
            _data[i][j].resize(dim + 1, 0.0);
        }
    }
    //std::cout << "done!" << std::endl;

    _boundingBoxVertices.push_back(glm::vec3(-gridSize/2.0, -gridSize/2.0, -gridSize/2.0) - gridCenter);
    _boundingBoxVertices.push_back(glm::vec3(-gridSize/2.0, -gridSize/2.0, gridSize/2.0) - gridCenter);
    _boundingBoxVertices.push_back(glm::vec3(-gridSize/2.0, gridSize/2.0, -gridSize/2.0) - gridCenter);
    _boundingBoxVertices.push_back(glm::vec3(-gridSize/2.0, gridSize/2.0, gridSize/2.0) - gridCenter);
    _boundingBoxVertices.push_back(glm::vec3(gridSize/2.0, -gridSize/2.0, -gridSize/2.0) - gridCenter);
    _boundingBoxVertices.push_back(glm::vec3(gridSize/2.0, -gridSize/2.0, gridSize/2.0) - gridCenter);
    _boundingBoxVertices.push_back(glm::vec3(gridSize/2.0, gridSize/2.0, -gridSize/2.0) - gridCenter);
    _boundingBoxVertices.push_back(glm::vec3(gridSize/2.0, gridSize/2.0, gridSize/2.0) - gridCenter);
    
    _boundingBoxIndices.push_back(0);
    _boundingBoxIndices.push_back(1);
    _boundingBoxIndices.push_back(5);
    _boundingBoxIndices.push_back(7);
    _boundingBoxIndices.push_back(3);
    _boundingBoxIndices.push_back(2);
    _boundingBoxIndices.push_back(6);
    _boundingBoxIndices.push_back(4);
    _boundingBoxIndices.push_back(0);
    _boundingBoxIndices.push_back(4);
    _boundingBoxIndices.push_back(5);
    _boundingBoxIndices.push_back(1);
    _boundingBoxIndices.push_back(3);
    _boundingBoxIndices.push_back(7);
    _boundingBoxIndices.push_back(6);
    _boundingBoxIndices.push_back(2);
    _boundingBoxIndices.push_back(0);

    
}

void VoxelData::generateData(const float noiseScale)
{
    //float startTime = glfwGetTime();    
    
    #pragma omp parallel for
    // Fill voxels with test-data, distance to center of volume.    
    for(unsigned x = 0; x < _dim + 1; x++)
    {
        for(unsigned y = 0; y < _dim + 1; y++)
        {
            for(unsigned z = 0; z < _dim + 1; z++)
            {
                //Create a plane at y = 0.
                _data[x][y][z] = (float)(y) / (float)(_dim + 1);


                for(int octave = 0; octave < 8; octave++)
                {
                    glm::vec3 pos = getWorldPosition(x,y,z) * noiseScale * (float)pow(2, octave);
                    float noise = snoise3(pos.x, pos.y, pos.z) * 0.25 * (1.0 / (pow(2, octave)));

                    _data[x][y][z] += noise;
                }
            }
        }
        
        /*if(omp_get_thread_num() == 0)
            std::cout << "Generating data " << (int)(((float)(x+1) * (float)omp_get_num_threads() / (float)_dim) * 100) << "%"
            << " on " << omp_get_num_threads() << " threads. Running time: " << glfwGetTime() - startTime << " seconds." << std::flush << "       \r";        
        */
    }
    //std::cout << std::endl;
}

void VoxelData::getInfo(bool showdata, bool printvertices, bool printnormals) const
{
    std::cout << std::endl;
    if(showdata)
        for(unsigned x = 0; x < _dim + 1; x++)
        {
            for(unsigned y = 0; y < _dim + 1; y++)
            {
                for(unsigned z = 0; z < _dim + 1; z++)
                {
                    std::cout << _data[x][y][z] << ", ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    
    if(printvertices)
        for(unsigned i = 0; i < _vertices.size(); i++)
            std::cout << "Vertice " << i << ": " << _vertices[i] << std::endl;

    if(printnormals)
        for(unsigned i = 0; i < _normals.size(); i++)
            std::cout << "Normal " << i << ": " << _normals[i] << std::endl;

    std::cout << "Vertices: " << _vertices.size() << std::endl;
    std::cout << "Normals: " << _normals.size() << std::endl;
    std::cout << "Indices: " << _indices.size() << std::endl;
}

void VoxelData::generateTriangles(const float isovalue)
{
    _isovalue = isovalue;
    //float startTime = glfwGetTime();

    #pragma omp parallel for        
    // Create triangles from the voxel data and the current isovalue.
    // Loop over all cubes in the volume.
    for (unsigned x = 0; x < _dim; x++)
    {
        for (unsigned y = 0; y < _dim; y++)
        {
            for (unsigned z = 0; z < _dim; z++)
            {
                unsigned triangleConfiguration = 0;

                // Compare the datapoints in one cube to the threshold.
                if (_data[x][y][z] > isovalue) triangleConfiguration |= 1;
                if (_data[x][y+1][z] > isovalue) triangleConfiguration |= 2;
                if (_data[x+1][y+1][z] > isovalue) triangleConfiguration |= 4;
                if (_data[x+1][y][z] > isovalue) triangleConfiguration |= 8;

                if (_data[x][y][z+1] > isovalue) triangleConfiguration |= 16;
                if (_data[x][y+1][z+1] > isovalue) triangleConfiguration |= 32;
                if (_data[x+1][y+1][z+1] > isovalue) triangleConfiguration |= 64;
                if (_data[x+1][y][z+1] > isovalue) triangleConfiguration |= 128;

                // Add vertices at the necessary edges, at the correct positions.
                for(unsigned n = 0; n < 16 && _table.triangleTable[triangleConfiguration][n] != -1; n += 3)
                {
                    unsigned e1 = _table.triangleTable[triangleConfiguration][n];
                    unsigned e2 = _table.triangleTable[triangleConfiguration][n+1];
                    unsigned e3 = _table.triangleTable[triangleConfiguration][n+2];
                    createTriangle(e1, e2, e3, x, y, z);
                }

            }
        }
        /*if(omp_get_thread_num() == 0)        
            std::cout << "Generating triangles " << (int)(((float)(x+1) * (float)omp_get_num_threads() / (float)_dim) * 100) << "%"
            << " on " << omp_get_num_threads() << " threads. Running time: " << glfwGetTime() - startTime << " seconds." << std::flush << "       \r";
        */
    }
    //std::cout << std::endl;    
            
    createVBO();
    createBuffers();
}

void VoxelData::createTriangle(unsigned e1, unsigned e2, unsigned e3, const unsigned x, const unsigned y, const unsigned z)
{

    unsigned edges[] = {e1, e2, e3};

    std::vector<glm::vec3> tempVert;
    std::vector<glm::vec3> tempNormal;

    // Loop through the three edges.
    for(unsigned i = 0; i < 3; i++)
    {
        unsigned v1, v2;

        // Determine which vertices v1 & v2 in a cube are connected to the current edge.
        if(edges[i] < 8)
        {
            v1 = edges[i];
            v2 = ((edges[i]+1) % 4) + ((edges[i] / 4) * 4);
        }
        else
        {
            v1 = edges[i] - 4;
            v2 = edges[i] - 8;
        }
        
        // Get the world position for the two vertices (not yet between 0 and 1).
        glm::ivec3 pos1 = getPosition(v1, x, y, z);
        glm::ivec3 pos2 = getPosition(v2, x, y, z);

        // Find the voxel value for the two vertices.
        float d1 = _data[pos1.x][pos1.y][pos1.z];
        float d2 = _data[pos2.x][pos2.y][pos2.z];

        // "Normalize" the positions so that the maximum value is 1 and minimum 0.
        glm::vec3 normalizedPos1 = ((glm::vec3)pos1 * (1.0f / (float)_dim)) * _gridSize;
        glm::vec3 normalizedPos2 = ((glm::vec3)pos2 * (1.0f / (float)_dim)) * _gridSize;

        // Interpolate between them with the given isovalue.
        glm::vec3 interpolatedPos = normalizedPos1 + ((normalizedPos2 - normalizedPos1) * ((_isovalue - d1) / (d2 - d1)));
        
        // Center the vertex (so that the whole grid is centered around origo) and add it to the array.
        glm::vec3 center = _gridCenter + glm::vec3(0.5f, 0.5f, 0.5f) * _gridSize;

        glm::vec3 normal = glm::vec3(0);
        // Calculate normal from a coarse estimation of the gradient
        for(int dx = -1; dx <= 1; dx++)
            for(int dy = -1; dy <= 1; dy++)
                for(int dz = -1; dz <= 1; dz++)
                {
                    int x1_clamped = clamp(pos1.x + dx, 0, _dim - 1);
                    int y1_clamped = clamp(pos1.y + dy, 0, _dim - 1);
                    int z1_clamped = clamp(pos1.z + dz, 0, _dim - 1);
                    normal += glm::vec3(dx, dy, dz) * _data[x1_clamped][y1_clamped][z1_clamped];                        

                    int x2_clamped = clamp(pos2.x + dx, 0, _dim - 1);
                    int y2_clamped = clamp(pos2.y + dy, 0, _dim - 1);
                    int z2_clamped = clamp(pos2.z + dz, 0, _dim - 1);
                    normal += glm::vec3(dx, dy, dz) * _data[x2_clamped][y2_clamped][z2_clamped];                                            
                }

        tempVert.push_back(interpolatedPos - center);
        tempNormal.push_back(normal);
    }

    #pragma omp critical
    {
        _vertices.insert(_vertices.end(), tempVert.begin(), tempVert.end());
        _normals.insert(_normals.end(), tempNormal.begin(), tempNormal.end());
    
        _indices.push_back(glm::ivec3(_vertices.size() - 3, _vertices.size() - 2, _vertices.size() - 1));
        
    }
}

const glm::ivec3 VoxelData::getPosition(const unsigned v, unsigned x, unsigned y, unsigned z) const
{
    if(v == 2 || v == 3 || v == 6 || v == 7)
        x++;
    if(v == 1 || v == 2 || v == 5 || v == 6)
        y++;
    if(v > 3)
        z++;

    return glm::vec3(x, y, z);
}

const glm::vec3 VoxelData::getWorldPosition(const unsigned x, const unsigned y, const unsigned z) const
{
    glm::vec3 pos = (glm::vec3(x,y,z) * (1.0f / (float)_dim)) * _gridSize;
    return pos - _gridCenter;
}

void VoxelData::draw() const
{
    glEnable(GL_CULL_FACE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _VBOarray.size(), &_VBOarray[0], GL_STATIC_DRAW);
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, sizeof(glm::ivec3) * _indices.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void VoxelData::drawBoundingBox() const
{
    glDisable(GL_CULL_FACE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_b);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _boundingBoxVertices.size(), &_boundingBoxVertices[0], GL_STATIC_DRAW);
    glBindVertexArray(VAO_b);

    glDrawElements(GL_LINES, sizeof(unsigned) * _boundingBoxIndices.size(), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}



void VoxelData::createVBO()
{
    for(unsigned i = 0; i < _vertices.size(); i++)
    {
        _VBOarray.push_back(_vertices[i]);
        _VBOarray.push_back(_normals[i]);
    }
}

void VoxelData::createBuffers()
{
    glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _VBOarray.size(), &_VBOarray[0], GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * _indices.size(), &_indices[0], GL_STATIC_DRAW);

	//Vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//Vertex normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (GLvoid*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);
	//Vertex color attribute
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (GLvoid*)(2 * sizeof(glm::vec3)));
	//glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    

    // Do the same for bounding box
    glGenVertexArrays(1, &VAO_b);
	glGenBuffers(1, &VBO_b);
	glGenBuffers(1, &EBO_b);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO_b);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_b);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _boundingBoxVertices.size(), &_boundingBoxVertices[0], GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_b);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * _boundingBoxIndices.size(), &_boundingBoxIndices[0], GL_STATIC_DRAW);

	//Vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 1 * sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
