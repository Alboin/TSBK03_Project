#include "voxelData.h"

// Function for printing glm::vec3 for debugging.
std::ostream& operator<<(std::ostream& os, const glm::vec3 &v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
} 

VoxelData::VoxelData(const unsigned dimx, const unsigned dimy, const unsigned dimz)
: _dim_x(dimx), _dim_y(dimy), _dim_z(dimz), _table(LookupTable())
{
    // Resize data-holder to the given dimensions.
    _data.resize(dimx);
    for(unsigned i = 0; i < dimy; i++)
    {
        _data[i].resize(dimy);
        for(unsigned j = 0; j < dimz; j++)
        {
            _data[i][j].resize(dimz);
        }
    }

}

void VoxelData::generateData(const unsigned seed)
{
    // Fill voxels with test-data, distance to center of volume.
    for(unsigned x = 0; x < _dim_x; x++)
    {
        for(unsigned y = 0; y < _dim_y; y++)
        {
            for(unsigned z = 0; z < _dim_z; z++)
            {
                //if(x == _dim_x/2 && y == _dim_y/2 && z == _dim_z/2)
                // if(x > _dim_x /4 && x < 3*_dim_x/4 && y > _dim_y /4 && y < 3*_dim_y/4 && z > _dim_z /4 && z < 3*_dim_z/4)
                //    _data[x][y][z] = 1.0;
                // else
                //    _data[x][y][z] = 0.0;

                float a = (float)(x - (_dim_x / 2)) / (_dim_x / 2);
                float b = (float)(y - (_dim_y / 2)) / (_dim_y / 2);
                float c = (float)(z - (_dim_z / 2)) / (_dim_z / 2);
                _data[x][y][z] = sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2));
            }
        }
    }

}

void VoxelData::getInfo(bool showdata, bool printvertices, bool printnormals) const
{
    if(showdata)
        for(unsigned x = 0; x < _dim_x; x++)
        {
            for(unsigned y = 0; y < _dim_y; y++)
            {
                for(unsigned z = 0; z < _dim_z; z++)
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

    // Create triangles from the voxel data and the current isovalue.
    // Loop over all cubes in the volume.
	for (unsigned x = 0; x < _dim_x - 1; x++)
		for (unsigned y = 0; y < _dim_y - 1; y++)
			for (unsigned z = 0; z < _dim_z - 1; z++)
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

                // Get the triangle configuration for this data.
                //std::vector<std::vector<unsigned>> triangles = _table.lookup(triangleConfiguration);

				// Add vertices at the necessary edges, at the correct positions.
                for(unsigned n = 0; n < 16 && _table.triangleTable[triangleConfiguration][n] != -1; n += 3)
                {
                    unsigned e1 = _table.triangleTable[triangleConfiguration][n];
                    unsigned e2 = _table.triangleTable[triangleConfiguration][n+1];
                    unsigned e3 = _table.triangleTable[triangleConfiguration][n+2];
                    createTriangle(e1, e2, e3, x, y, z);
                }
                    
                //for (unsigned n = 0; n < triangles.size(); n++)
				//	createTriangle(triangles[n], x, y, z);

            }
            
    calculateNormals();
    createVBO();
    createBuffers();
}

void VoxelData::createTriangle(unsigned e1, unsigned e2, unsigned e3, const unsigned x, const unsigned y, const unsigned z)
{

    unsigned edges[] = {e1, e2, e3};

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
            /*if(edges[i] == 8)
                v1 = 1, v2 = 5;
            else if(edges[i] == 9)
                v1 = 2, v2 = 6;
            else if(edges[i] == 10)
                v1 = 0, v2 = 0;
            else if(edges[i] == 11)
                v1 = 3, v2 = 7;*/
        }
        
        // Get the world position for the two vertices (not yet between 0 and 1).
        glm::ivec3 pos1 = getPosition(v1, x, y, z);
        glm::ivec3 pos2 = getPosition(v2, x, y, z);

        // Find the voxel value for the two vertices.
        float d1 = _data[pos1.x][pos1.y][pos1.z];
        float d2 = _data[pos2.x][pos2.y][pos2.z];

        // "Normalize" the positions so that the maximum value is 1 and minimum 0.
        glm::vec3 normalizedPos1 = glm::vec3((float)pos1.x / (float)_dim_x, (float)pos1.y / (float)_dim_y, (float)pos1.z / (float)_dim_z);
        glm::vec3 normalizedPos2 = glm::vec3((float)pos2.x / (float)_dim_x, (float)pos2.y / (float)_dim_y, (float)pos2.z / (float)_dim_z);

        // Interpolate between them with the given isovalue.
        glm::vec3 interpolatedPos = normalizedPos1 + ((normalizedPos2 - normalizedPos1) * ((_isovalue - d1) / (d2 - d1)));
        
        // Center the vertex (so that the whole grid is centered around origo) and add it to the array.
        glm::vec3 center = glm::vec3(0.5f, 0.5f, 0.5f);
        _vertices.push_back((interpolatedPos  - center) * 15.0f);
    }

    _indices.push_back(glm::ivec3(_vertices.size() - 3, _vertices.size() - 2, _vertices.size() - 1));
}

const glm::ivec3 VoxelData::getPosition(const unsigned v, unsigned x, unsigned y, unsigned z) const
{
    if(v == 2 || v == 3 || v == 6 || v == 7)
        x++;
    if(v == 1 || v == 2 || v == 5 || v == 6)
        y++;
    if(v > 3)
        z++;

    //glm::vec3 temp = glm::vec3((float)x / (float)_dim_x, (float)y / (float)_dim_y, (float)z / (float)_dim_z);
        
    //std::cout << (float)x << " : " << (float)_dim_x << " : " << (float)x / (float)_dim_x << temp << std::endl;
    
    return glm::vec3(x, y, z);
}



const unsigned VoxelData::hashFunction(const unsigned x, const unsigned y, const unsigned z) const
{
    return 1;
}

void VoxelData::calculateNormals()
{
    _normals.resize(_vertices.size());
    for(unsigned i = 0; i < _indices.size(); i++)
    {
        glm::vec3 a = _vertices[_indices[i].y] - _vertices[_indices[i].x];
        glm::vec3 b = _vertices[_indices[i].z] - _vertices[_indices[i].x];
        glm::vec3 normal = glm::normalize(glm::cross(a, b));

        _normals[_indices[i].x] = normal;
        _normals[_indices[i].y] = normal;
        _normals[_indices[i].z] = normal;
    }
}

void VoxelData::draw() const
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _VBOarray.size(), &_VBOarray[0], GL_STATIC_DRAW);
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, sizeof(glm::ivec3) * _indices.size(), GL_UNSIGNED_INT, 0);

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
}
