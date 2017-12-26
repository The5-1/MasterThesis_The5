#pragma once
//Extern
#include <glm\glm.hpp>
#include <GL/glew.h>
#include <GL/glut.h>
#include "helper.h"
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
	int boxColorId;

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

	//OpenGL PointCloud
	GLuint vboPC[5];
	std::vector<int> vertexIndexList;
	std::vector<glm::vec3> vertexColorList; //For Debug, delete later

	//OpenGL Box
	GLuint vboBox[2];
	std::vector<glm::vec3> boxVertices;
	std::vector<unsigned int> boxIndices;

	//Max size
	int maxVerticesPerQuad;

	//Debug variables
	std::vector<glm::mat4> modelMatrixLowestLeaf;
	std::vector<glm::vec3> colorLowestLeaf;
	std::vector<glm::vec3> colorOctree = { glm::vec3(1.0f, 0.0f, 0.0f),		//Red		
											glm::vec3(1.0f, 0.6f, 0.0f),	//Orange
											glm::vec3(0.0f, 0.5f, 0.5f),	//Teal
											glm::vec3(0.5f, 0.5f, 0.65f),	//Blue-grey		
											glm::vec3(0.0f, 1.0f, 0.0f),	//Green		
											glm::vec3(0.5f, 0.2f, 0.05f),	//Brown
											glm::vec3(1.0f, 0.0f, 1.0f),	//Purple
											glm::vec3(0.0f, 0.0f, 1.0f),	//Blue	
										};
	

//Functions
public:
	PC_Octree();
	PC_Octree(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _normals, std::vector<float>& _radius, int _maxVerticesPerQuad);

	PC_Octree(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _normals, std::vector<glm::vec3>& _colors, std::vector<float>& _radius, int _maxVerticesPerQuad);

	~PC_Octree();

	void getAabbUniforms(glm::mat4& _modelMatrix);
	void getAabbLeafUniforms(glm::mat4 & _modelMatrix, Octree _leaf);

	void drawBox();

	void uploadPointCloud(std::vector<glm::vec3>& _vertices, std::vector<glm::vec3>& _normals, std::vector<float>& _radius);
	void drawPointCloud();

	bool onCorrectPlaneSide(glm::vec3& corner, glm::vec3& normal, glm::vec3& point);

	int boxFrstrumCull(Octree & leaf, glm::vec3 & normal, glm::vec3 & point);

	void cullWithViewFrustrum(Octree& leaf, viewFrustrum& vF);

	void addBoxToDraw(Octree & leaf, glm::vec3 color);

	void initViewFrustrumCull(Octree & leaf, viewFrustrum & vF);

private:
	void splitLeaf(Octree& leaf, std::vector<glm::vec3>& _vertices);
	void copyIndexVector(std::vector<int>& leafIndex, int offset);
	void getAABB(glm::vec3& min, glm::vec3& max, std::vector<glm::vec3>& _vertices);
	void getAABB(glm::vec3 & min, glm::vec3& max, std::vector<glm::vec3>& _vertices, std::vector<int> _indices);
	void uploadGlBox();
	
};

