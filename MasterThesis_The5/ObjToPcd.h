#pragma once
#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

void objToPcd(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals) {
	if(vertices.size() != normals.size()){
		std::cout << "ERROR: Vertices size (" << vertices.size() << ") does not match normals size (" << normals.size() << ")" << std::endl;
	}

	//Create new file
	std::ofstream file;
	file.open("C:/Users/Kompie8/Documents/Visual Studio 2015/Projects/MasterThesis_The5/MasterThesis_The5/pointclouds/selfmadeTeapot.pcd");

	file << "# .PCD v.7 - Point Cloud Data file format\n";
	file << "VERSION .7\n";
	file << "FIELDS x y z normal_x normal_y normal_z\n";
	file << "SIZE 4 4 4 4 4 4\n";
	file << "TYPE F F F F F F\n";
	file << "COUNT 1 1 1 1 1 1\n";
	file << "WIDTH " << vertices.size() << "\n";
	file << "HEIGHT 1\n";
	file << "VIEWPOINT 0 0 0 1 0 0 0\n";
	file << "POINTS " << vertices.size() << "\n";
	file << "DATA ascii\n";
	for (int i = 0; i < vertices.size(); i++) {
		file << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << normals[i].x << " " << normals[i].y << " " << normals[i].z << "\n";
	}

	//Close file
	file.close();
}