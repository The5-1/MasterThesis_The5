#pragma once
//Extern
#include <glm\glm.hpp>
#include <GL/glew.h>
#include <GL/glut.h>

//Std
#include <iostream>
#include <vector>
#include <bitset>

struct Octree {
public:
	int beginVertices;
	int endVertices;

	//glm::vec3 averageNormal;
	//glm::vec3 averageColor;

	std::bitset<8> bitMaskChildren; // [0,0,0,0,0,0,0,0], set to 1 if the corresponding child exists
	std::vector<Octree> children;

	//Experimental: Helper variables to draw a box around the leaf
	glm::vec3 minLeafBox, maxLeafBox;

	Octree() {
	}

	Octree(int _beginVertices, int _endVertices) {
		this->beginVertices = _beginVertices;
		this->endVertices = _endVertices;
	}

	Octree(int _beginVertices, int _endVertices, glm::vec3 _minLeafBox, glm::vec3 _maxLeafBox) {
		this->beginVertices = _beginVertices;
		this->endVertices = _endVertices;
		this->maxLeafBox = _maxLeafBox;
		this->minLeafBox = _minLeafBox;
	}


};


class PC_Octree
{
//Variables
public:
	Octree root;

	//AABB
	glm::vec3 minBoundingBox, maxBoundingBox;

	//Index-List
	std::vector<int> vertexIndexList;

	//OpenGL Box
	GLuint vboBox[2];
	std::vector<glm::vec3> boxVertices;
	std::vector<unsigned int> boxIndices;

	//Max size
	int maxVerticesPerQuad;
	//Debug variables
	std::vector<glm::mat4> modelMatrixLowestLeaf;


//Functions
public:
	PC_Octree();
	PC_Octree(std::vector<glm::vec3>& _vertices, int _maxVerticesPerQuad);

	~PC_Octree();

	void getAabbUniforms(glm::mat4& _modelMatrix);
	void getAabbLeafUniforms(glm::mat4 & _modelMatrix, Octree _leaf);
	void drawBox();


private:
	void splitLeaf(Octree& leaf, std::vector<glm::vec3>& _vertices);
	void getAABB(glm::vec3& min, glm::vec3& max, std::vector<glm::vec3>& _vertices);
	void getAABB(glm::vec3 & min, glm::vec3& max, std::vector<glm::vec3>& _vertices, std::vector<int> _indices);
	void uploadGlBox();
	
};

