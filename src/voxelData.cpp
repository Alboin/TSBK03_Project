#include "voxelData.h"

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
                _data[x][y][z] = sqrt(pow((float)x / _dim_x, 2) + pow((float)y / _dim_y, 2) + pow((float)z / _dim_z, 2));
            }
        }
    }
}

void VoxelData::getInfo(bool showdata) const
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
				if (_data[x][y][z] > isovalue)
					triangleConfiguration += pow(2, 0);
				if (_data[x+1][y][z] > isovalue)
					triangleConfiguration += pow(2, 1);
				if (_data[x][y+1][z] > isovalue)
					triangleConfiguration += pow(2, 2);
				if (_data[x+1][y+1][z] > isovalue)
                    triangleConfiguration += pow(2, 3);

				if (_data[x][y][z+1] > isovalue)
					triangleConfiguration += pow(2, 4);
                if (_data[x+1][y][z+1] > isovalue)
					triangleConfiguration += pow(2, 5);
                if (_data[x][y+1][z+1] > isovalue)
					triangleConfiguration += pow(2, 6);
                if (_data[x+1][y+1][z+1] > isovalue)
					triangleConfiguration += pow(2, 7);

				// Get the triangle configuration for this data.
                std::vector<std::vector<unsigned>> triangles = _table.lookup(triangleConfiguration);

				// Add vertices at the necessary edges, at the correct positions.
                for (unsigned n = 0; n < triangles.size(); n++)
					createTriangle(triangles[n], x, y, z);

            }
            
    calculateNormals();
    createVBO();
    createBuffers();
}

void VoxelData::createTriangle(std::vector<unsigned> edges, const unsigned x, const unsigned y, const unsigned z)
{

    // Loop through the three edges.
    for(unsigned i = 0; i < edges.size(); i++)
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
            v1 = edges[i] - 8;
            v2 = edges[i] - 4;
        }
        
        // Get the world position for the two vertices.
        glm::ivec3 pos1 = getPosition(v1, x, y, z);
        glm::ivec3 pos2 = getPosition(v2, x, y, z);

        // Find the voxel value for the two vertices.
        float d1 = _data[pos1.x][pos1.y][pos1.z];
        float d2 = _data[pos2.x][pos2.y][pos2.z];

        // Interpolate between them with the given isovalue.
        glm::vec3 interpolatedPos = (glm::vec3)pos1 + (glm::vec3)(pos2 - pos1) * ((_isovalue - d1) / (d2 - d1));

        _vertices.push_back(interpolatedPos);
    }

    _indices.push_back(glm::ivec3(_vertices.size() - 3, _vertices.size() - 2, _vertices.size() - 1));
}

const glm::ivec3 VoxelData::getPosition(const unsigned v, unsigned x, unsigned y, unsigned z) const
{

    if(v == 1 || v == 2 || v == 5 || v == 6)
        x++;
    if(v > 3)
        y++;
    if(v == 2 || v == 3 || v == 6 || v == 7)
        z++;

    return glm::vec3(x,y,z);
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
        glm::vec3 normal = _vertices[_indices[i].x] + _vertices[_indices[i].y] + _vertices[_indices[i].z];
        normal = glm::normalize(normal);

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


