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
		file <<"v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << " " << normals[i].x << " " << normals[i].y << " " << normals[i].z << "\n";
	}

	//Close file
	file.close();
}

void loadBigFile(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<float>& radii, const char *filename) {
	
	int counter = 0;

	vertices.clear();
	normals.clear();
	radii.clear();

	FILE * file = fopen(filename, "r");
	if (file == NULL) {
		cerr << "Model file not found: " << filename << endl;
		exit(0);
	}

	while (1)
	{
		counter++;
		if (counter > 10000) {
			std::cout << "Break loading big file due to memory shortage" << std::endl;
			break;
		}

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "v") == 0)
		{
			glm::vec3 vertex;
			glm::vec3 normal;
			float radius;

			fscanf(file, "%f %f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y, &normal.z, &radius);
			vertices.push_back(vertex);
			normals.push_back(normal);
			radii.push_back(radius);

			//std::cout << vertex.x << " " << vertex.y << " " << vertex.z << " " << normal.x << " " << normal.y << " " << normal.z << " " << radius << std::endl;
		}

		
	}

}