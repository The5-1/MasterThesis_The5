#define GLEW_STATIC //Using the static lib, so we need to enable it

#include "PC_Octree.h"

#include <glm/gtc/matrix_transform.hpp> 
#include <algorithm>

/* *********************************************************************************************************
//Functions (public)
********************************************************************************************************* */
PC_Octree::PC_Octree()
{
}

PC_Octree::PC_Octree(std::vector<glm::vec3>& _vertices, int _maxVerticesPerQuad)
{

	this->getAABB(minBoundingBox, maxBoundingBox, _vertices);

	this->uploadGlBox();

	this->root.endVertices = _vertices.size();
	this->root.beginVertices = 0;

	this->maxVerticesPerQuad = _maxVerticesPerQuad;

	this->root.maxLeafBox = this->maxBoundingBox;
	this->root.minLeafBox = this->minBoundingBox;

	for (int i = 0; i < this->root.endVertices; i++) {
		this->vertexIndexList.push_back(i);
	}


	if (_vertices.size() < _maxVerticesPerQuad) {
		std::cout << "Model has less vertices then give max size per leaf. All informations stored in root." << std::endl;
		return;
	}

	this->splitLeaf(this->root, _vertices);



}

PC_Octree::~PC_Octree()
{
}


void PC_Octree::getAabbUniforms(glm::mat4 & _modelMatrix)
{
	_modelMatrix = glm::mat4(1.0f);
	
	_modelMatrix = glm::translate(_modelMatrix, glm::vec3(minBoundingBox));

	glm::vec3 scaleVec = glm::abs(maxBoundingBox - minBoundingBox);

	_modelMatrix = glm::scale(_modelMatrix, scaleVec);
}

void PC_Octree::getAabbLeafUniforms(glm::mat4 & _modelMatrix, Octree _leaf)
{
	_modelMatrix = glm::mat4(1.0f);

	_modelMatrix = glm::translate(_modelMatrix, glm::vec3(_leaf.minLeafBox));

	glm::vec3 scaleVec = glm::abs(_leaf.maxLeafBox - _leaf.minLeafBox);

	_modelMatrix = glm::scale(_modelMatrix, scaleVec);
}

void PC_Octree::drawBox()
{
	//Enable wireframe mode
	glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);

	//Draw vertices as glQuads
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboBox[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboBox[1]);
	glDrawElements(GL_QUADS, this->boxIndices.size(), GL_UNSIGNED_INT, 0);

	//Disable wireframe mode
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
}

/* *********************************************************************************************************
//Functions (private)
********************************************************************************************************* */

/********************************		OCTREE creation ****************************************************
Names of new octree-Boxes
  /----------------/|	
 /-------/--------/ |
/_______/________/| | 
|		|		| | |
|000	|010	| |/|
|		|		| / |
-----------------/| |
|		|		| | |
|100	|110	| |/
|		|		| /
-----------------/

Coordinate System (OpenGL standard)
y-Direction
|
|  / z-Direction 
| /	
|/
-------- x-Direction

Octree-Split-Names-Convention, the new boxes in z-Direction are labeled 001, 011,...

*/



void PC_Octree::splitLeaf(Octree& leaf, std::vector<glm::vec3>& _vertices) {
	{
		std::vector<int> newLeaf000, newLeaf010, newLeaf100, newLeaf110, newLeaf001, newLeaf011, newLeaf101, newLeaf111;
		glm::vec3 splittingPlain = leaf.minLeafBox + 0.5f * (leaf.maxLeafBox - leaf.minLeafBox);

		glm::vec3 splitDiagonal = leaf.maxLeafBox - leaf.minLeafBox;


		for (int i = leaf.beginVertices; i < leaf.endVertices; i++) {

			glm::vec3 currentVertex = _vertices[i];

			if (currentVertex.x <= splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf000.push_back(i);
			}

			else if (currentVertex.x <= splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf100.push_back(i);
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf010.push_back(i);
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z <= splittingPlain.z) {
				newLeaf110.push_back(i);
			}

			else if (currentVertex.x <= splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf001.push_back(i);
			}

			else if (currentVertex.x <= splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf101.push_back(i);
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y > splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf011.push_back(i);
			}

			else if (currentVertex.x > splittingPlain.x && currentVertex.y <= splittingPlain.y && currentVertex.z > splittingPlain.z) {
				newLeaf111.push_back(i);
			}
		}

		int currentStart = leaf.beginVertices;

		//Front
		if (newLeaf000.size() > 0) {
			leaf.bitMaskChildren[0] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.0f, 0.5f * splitDiagonal.y, 0.0f);
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, splitDiagonal.y, 0.5f * splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf000);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf000.size(), min, max));

			std::copy(newLeaf000.begin(), newLeaf000.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf000.size();
		}

		if (newLeaf010.size() > 0) {
			leaf.bitMaskChildren[1] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.0f);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, splitDiagonal.y, 0.5f * splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf010);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf010.size(), min, max));

			std::copy(newLeaf010.begin(), newLeaf010.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf010.size();
		}

		if (newLeaf100.size() > 0) {
			leaf.bitMaskChildren[2] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox;
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf100);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf100.size(), min, max));

			std::copy(newLeaf100.begin(), newLeaf100.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf100.size();
		}

		if (newLeaf110.size() > 0) {
			leaf.bitMaskChildren[3] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.0f, 0.0f);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf110);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf110.size(), min, max));

			std::copy(newLeaf110.begin(), newLeaf110.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf110.size();
		}

		//Back
		if (newLeaf001.size() > 0) {
			leaf.bitMaskChildren[4] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.0f, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, splitDiagonal.y, splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf000);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf001.size(), min, max));

			std::copy(newLeaf001.begin(), newLeaf001.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf001.size();
		}

		if (newLeaf011.size() > 0) {
			leaf.bitMaskChildren[5] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, 0.5f * splitDiagonal.z);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, splitDiagonal.y, splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf010);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf011.size(), min, max));

			std::copy(newLeaf011.begin(), newLeaf011.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf011.size();
		}

		if (newLeaf101.size() > 0) {
			leaf.bitMaskChildren[6] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.0f, 0.0f, 0.5f * splitDiagonal.z);;
			max = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.5f * splitDiagonal.y, splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf100);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf101.size(), min, max));

			std::copy(newLeaf101.begin(), newLeaf101.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf101.size();
		}

		if (newLeaf111.size() > 0) {
			leaf.bitMaskChildren[7] = 1;
			glm::vec3 min, max;

			min = leaf.minLeafBox + glm::vec3(0.5f * splitDiagonal.x, 0.0f, 0.5f * splitDiagonal.z);
			max = leaf.minLeafBox + glm::vec3(splitDiagonal.x, 0.5f * splitDiagonal.y, splitDiagonal.z);
			//this->getAABB(min, max, _vertices, newLeaf110);

			leaf.children.push_back(Octree(currentStart, currentStart + newLeaf111.size(), min, max));

			std::copy(newLeaf111.begin(), newLeaf111.end(), vertexIndexList.begin() + leaf.beginVertices + currentStart);

			currentStart += newLeaf111.size();
		}
	}

	//Debug Model Matrix
	if (leaf.bitMaskChildren[0] == 0 && leaf.bitMaskChildren[1] == 0 && leaf.bitMaskChildren[2] == 0 && leaf.bitMaskChildren[3] == 0 && leaf.bitMaskChildren[4] == 0 && leaf.bitMaskChildren[5] == 0 && leaf.bitMaskChildren[6] == 0 && leaf.bitMaskChildren[7] == 0) {
		glm::mat4 modelMatrix;
		this->getAabbLeafUniforms(modelMatrix, leaf);
		modelMatrixLowestLeaf.push_back(modelMatrix);
	}
	/*
	DebugMe: This will thow a stack overflow due to low stack size
	*/
	//for (int i = 0; i < leaf.children.size(); i++) {

	//	std::cout << leaf.children[i].endVertices - leaf.children[i].beginVertices << std::endl;

	//	if (leaf.children[i].endVertices - leaf.children[i].beginVertices > this->maxVerticesPerQuad) {
	//		this->splitLeaf(leaf.children[i], _vertices);
	//	}
	//}
}

void PC_Octree::getAABB(glm::vec3& min, glm::vec3& max, std::vector<glm::vec3>& _vertices)
{
	min = _vertices[0];
	max = _vertices[0];

	for (int i = 1; i < _vertices.size(); i++) {

		if (_vertices[i].x < min.x) {
			min.x = _vertices[i].x;
		}

		if (_vertices[i].y < min.y) {
			min.y = _vertices[i].y;
		}

		if (_vertices[i].z < min.z) {
			min.z = _vertices[i].z;
		}

		if (_vertices[i].x > max.x) {
			max.x = _vertices[i].x;
		}

		if (_vertices[i].y > max.y) {
			max.y = _vertices[i].y;
		}

		if (_vertices[i].z > max.z) {
			max.z = _vertices[i].z;
		}

	}
}

void PC_Octree::getAABB(glm::vec3& min, glm::vec3& max, std::vector<glm::vec3>& _vertices, std::vector<int> _indices)
{
	min = _vertices[_indices[0]];
	max = _vertices[_indices[0]];

	for (int i = 1; i < _indices.size(); i++) {

		if (_vertices[_indices[i]].x < min.x) {
			min.x = _vertices[_indices[i]].x;
		}

		if (_vertices[_indices[i]].y < min.y) {
			min.y = _vertices[_indices[i]].y;
		}

		if (_vertices[_indices[i]].z < min.z) {
			min.z = _vertices[_indices[i]].z;
		}

		if (_vertices[_indices[i]].x > max.x) {
			max.x = _vertices[_indices[i]].x;
		}

		if (_vertices[_indices[i]].y > max.y) {
			max.y = _vertices[_indices[i]].y;
		}

		if (_vertices[_indices[i]].z > max.z) {
			max.z = _vertices[_indices[i]].z;
		}

	}
}

void PC_Octree::uploadGlBox()
{
	boxVertices = { glm::vec3(1.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(1.0, 0.0, 0.0),
		glm::vec3(1.0, 1.0, 0.0),
		glm::vec3(1.0, 1.0, 1.0),
		glm::vec3(0.0, 1.0, 1.0),
		glm::vec3(0.0, 1.0, 0.0)
	};

	boxIndices = { 0,1,2,3,
					0,3,4,5,
					0,5,6,1,
					1,6,7,2,
					7,4,3,2,
					4,7,6,5 };


	glGenBuffers(2, this->vboBox);
	glBindBuffer(GL_ARRAY_BUFFER, this->vboBox[0]);
	glBufferData(GL_ARRAY_BUFFER, this->boxVertices.size() * sizeof(float) * 3, this->boxVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboBox[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->boxIndices.size() * sizeof(unsigned int), this->boxIndices.data(), GL_STATIC_DRAW);
}


