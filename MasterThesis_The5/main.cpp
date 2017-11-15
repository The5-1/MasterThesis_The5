#define GLEW_STATIC //Using the static lib, so we need to enable it
#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <Ant/AntTweakBar.h>
#include <memory>
#include <algorithm>
#include "helper.h"
#include "Shader.h"
#include "Skybox.h"
#include "times.h"
#include "InstancedMesh.h"
#include "FBO.h"
#include "Texture.h"
#include "glm/gtx/string_cast.hpp"
#include "marchingCubesVolume.h"

#include "HalfEdgeMesh.h"
#include "MeshResampler.h"

//Time
Timer timer;
int frame;
long timeCounter, timebase;
char timeString[50];

//Resolution (has to be changed in helper.h too)
glm::vec2 resolution = glm::vec2(1024, 768);

//Externals
cameraSystem cam(1.0f, 1.0f, glm::vec3(20.95f, 20.95f, -0.6f));
glm::mat4 projMatrix;
glm::mat4 viewMatrix;

bool leftMouseClick;
int leftMouseClickX;
int leftMouseClickY;

//Skybox
Skybox skybox;
char* negz = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negz.jpg";
char* posz = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posz.jpg";
char* posy = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posy.jpg";
char* negy = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negy.jpg";
char* negx = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negx.jpg";
char* posx = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posx.jpg";

//Shaders
Shader basicShader;
Shader modelLoaderShader;

//Skybox
Shader skyboxShader;

//Models
simpleModel *teaPot = 0;
HalfedgeMesh heMesh;

// tweak bar
TwBar *tweakBar;
bool wireFrameTeapot = true;

glm::vec3 lightDir;

/* *********************************************************************************************************
TweakBar
********************************************************************************************************* */
void setupTweakBar() {
	TwInit(TW_OPENGL_CORE, NULL);
	tweakBar = TwNewBar("Settings");
	//TwAddVarRW(tweakBar, "lightDirection", TW_TYPE_DIR3F, &lightDir, "label='Light Direction'");
	TwAddSeparator(tweakBar, "Wireframe", nullptr);
	TwAddVarRW(tweakBar, "Wireframe Teapot", TW_TYPE_BOOLCPP, &wireFrameTeapot, " label='Wireframe Teapot' ");
	//// Array of drop down items
	//TwEnumVal Operations[] = { { SPLIT, "SPLIT" },{ FLIP, "FLIP" },{ COLLAPSE, "COLLAPSE" }};
	//// ATB identifier for the array
	//TwType TwOperations = TwDefineEnum("Operations", Operations, 3);
	//// Link it to the tweak bar
	//TwAddVarRW(tweakBar, "Operations", TwOperations, &m_operation, NULL);
}

/* *********************************************************************************************************
Initiation
********************************************************************************************************* */
std::vector<unsigned int> indices;
std::vector<glm::vec3> vertices;
GLuint vbo[2];

void load_VTKfile() {
	char* filename = "mesh.vtk";

	FILE * file = fopen(filename, "r");

	if (file == NULL) {
		cerr << "Model file not found: " << filename << endl;
		exit(0);
	}

	while (1)
	{
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "POINTS") == 0)
		{
			int numPoints;
			fscanf(file, "%d %s \n", &numPoints);
			for (int i = 0; i < numPoints; i++) {
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				vertices.push_back(vertex);
				//std::cout << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
			}
		}

		if (strcmp(lineHeader, "POLYGONS") == 0)
		{
			int numPolygons;
			int numPolygonsTimeFour;

			fscanf(file, "%d %d \n", &numPolygons, &numPolygonsTimeFour);
			for (int i = 0; i < numPolygons; i++) {
				glm::vec3 vertex;
				int three, index0, index1, index2;
				fscanf(file, "%d %d %d %d\n", &three, &index0, &index1, &index2);
				indices.push_back(index0);
				indices.push_back(index1);
				indices.push_back(index2);
				//std::cout << index0 << " " << index1 << " " << index2 << std::endl;
			}
		}
	}

	fclose(file);
	
}

void upload_VTKfile() {
	glGenBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void draw_VTKfile() {
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void delete_VTKfile(){

}

void init() {
	load_VTKfile();
	upload_VTKfile();
	/*****************************************************************
	Skybox (Only for aesthetic reasons, can be deleted)
	*****************************************************************/
	skybox.createSkybox(negz, posz, posy, negy, negx, posx);
}


void loadShader(bool init) {
	basicShader = Shader("./shader/basic.vs.glsl", "./shader/basic.fs.glsl");
	modelLoaderShader = Shader("./shader/modelLoader.vs.glsl", "./shader/modelLoader.fs.glsl");
	skyboxShader = Shader("./shader/skybox.vs.glsl", "./shader/skybox.fs.glsl");
}

/* *********************************************************************************************************
Scenes: Unit cube + Pointcloud, Results of marching cubes
********************************************************************************************************* */
void sponzaStandardScene(){
	skyboxShader.enable();
	skyboxShader.uniform("projMatrix", projMatrix);
	skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	skybox.Draw(skyboxShader);
	skyboxShader.disable();

	

	/* ********************************************
	Half-Edge-Mesh
	**********************************************/

	basicShader.enable();
	if (wireFrameTeapot) {
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}

	glm::mat4 modelMatrix = glm::scale(glm::vec3(50.0f));

	basicShader.uniform("modelMatrix", modelMatrix);
	basicShader.uniform("viewMatrix", viewMatrix);
	basicShader.uniform("projMatrix", projMatrix);

	basicShader.uniform("col", glm::vec3(1.0f, 0.0f, 0.0f));

	draw_VTKfile();

	if (wireFrameTeapot) {
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}
	basicShader.disable();
}	
/* *********************************************************************************************************
Vector/Triangle - Intersection
********************************************************************************************************* */
//float rayIntersectsTriangle(glm::vec3 origin, glm::vec3 direction, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, float& u_out, float& v_out) {
//	//Intersection - Test:
//	//http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
//	glm::vec3 h, s, q;
//	float a, f, u, v;
//
//	glm::vec3 e1 = v1 - v0;
//	glm::vec3 e2 = v2 - v0;
//
//	h = glm::cross(direction, e2);
//	a = glm::dot(e1, h);
//
//	if (a > -0.00001f && a < 0.00001f)
//		return(-1.0f);
//
//	f = 1.0f / a;
//	s = origin - v0;
//	u = f * (glm::dot(s, h));
//
//	if (u < 0.0f || u > 1.0f)
//		return(-1.0f);
//
//	q = glm::cross( s, e1);
//	v = f * glm::dot(direction, q);
//
//	if (v < 0.0f || u + v > 1.0f)
//		return(-1.0f);
//
//	// at this stage we can compute t to find out where
//	// the intersection point is on the line
//	float t = f * glm::dot(e2, q);
//
//	if (t > 0.00001f) { // ray intersection
//		u_out = u;
//		v_out = v;
//		return(t);
//	}
//
//	else // this means that there is a line intersection
//		 // but not a ray intersection
//		return (-1.0f);
//}
//
///*********************************************
//Mouse selection on simpleModel types (vector of vertices/indices)
//*********************************************/
//void mouseTriangleSelecction() {
//	if (leftMouseClick) {
//
//		leftMouseClick = false;
//
//		float x = 2.0f * (float(leftMouseClickX) / float(WIDTH)) - 1.0f;
//		float y = 1.0f - 2.0f * (float(leftMouseClickY) / float(HEIGHT));
//
//		glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
//
//		glm::vec4 ray_eye = glm::inverse(projMatrix) * ray_clip;
//		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
//
//		glm::vec3 ray_wor = glm::vec3((glm::inverse(viewMatrix) * ray_eye));
//		ray_wor = glm::normalize(ray_wor);
//
//		vector<pair<float, pair<int, FaceIter>>> list;
//		int i = 0;
//		for (FaceIter f = heMesh.facesBegin(); f != heMesh.facesEnd(); f++) {
//
//			glm::mat4 modelMatrixIntersect = glm::mat4(1.0f);
//			modelMatrixIntersect = glm::translate(modelMatrixIntersect, glm::vec3(-10.0f, 0.0f, 0.0f));
//			modelMatrixIntersect = glm::scale(modelMatrixIntersect, glm::vec3(3.0f));
//
//			HalfedgeIter h = f->halfedge();
//			glm::vec3 v0 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//			h = h->next();
//			glm::vec3 v1 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//			h = h->next();
//			glm::vec3 v2 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//			float u, v;
//			float t = rayIntersectsTriangle(cam.position, ray_wor, v0, v1, v2, u, v);
//
//
//			if (t != -1.0f) {
//				std::cout << "t: " << t << " u: " << u << " v: " << v << std::endl;
//				list.push_back(pair<float, pair<int, FaceIter>>(t, pair<int, FaceIter>(i, f)));
//			}
//
//			i++;
//		}
//
//
//
//		//If we dont even hit a triangle stop here
//		if (!list.empty()) {
//
//			//Sort all triangles hit by ray
//			sort(list.begin(), list.end());
//
//			//Find the closest edge to the selected point
//			//We need the barycentric coordinates of the intersection ray/triangle ( we could get this smarter then recalculating)
//			FaceIter f_sel = list[0].second.second;
//			glm::mat4 modelMatrixIntersect = glm::mat4(1.0f);
//			modelMatrixIntersect = glm::translate(modelMatrixIntersect, glm::vec3(-10.0f, 0.0f, 0.0f));
//			modelMatrixIntersect = glm::scale(modelMatrixIntersect, glm::vec3(3.0f));
//			HalfedgeIter h = f_sel->halfedge();
//			glm::vec3 v0 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//			h = h->next();
//			glm::vec3 v1 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//			h = h->next();
//			glm::vec3 v2 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//			float u = -1.0f, v = -1.0f;
//			float t = rayIntersectsTriangle(cam.position, ray_wor, v0, v1, v2, u, v);
//			float w = 1.0f - u - v;
//
//			halfEdgeMeshColors.erase(halfEdgeMeshColors.begin(), halfEdgeMeshColors.end());
//			halfEdgeMeshVertices.erase(halfEdgeMeshVertices.begin(), halfEdgeMeshVertices.end());
//
//			for (FaceIter f = heMesh.facesBegin(); f != heMesh.facesEnd(); f++) {
//				//1. Half Edge of current face
//				HalfedgeIter h = f->halfedge();
//				halfEdgeMeshVertices.push_back(glm::vec3(h->vertex()->position.x, h->vertex()->position.y, h->vertex()->position.z));
//				
//				//2. Half Edge of current face
//				h = h->next();
//				halfEdgeMeshVertices.push_back(glm::vec3(h->vertex()->position.x, h->vertex()->position.y, h->vertex()->position.z));
//
//				//3. Half Edge of current face
//				h = h->next();
//				halfEdgeMeshVertices.push_back(glm::vec3(h->vertex()->position.x, h->vertex()->position.y, h->vertex()->position.z));
//
//				if (f_sel == f) {
//					halfEdgeMeshColors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
//					halfEdgeMeshColors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
//					halfEdgeMeshColors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
//				}
//				else {
//					halfEdgeMeshColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
//					halfEdgeMeshColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
//					halfEdgeMeshColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
//				}
//			}
//
//			glGenBuffers(2, vboHalfEdgeMesh);
//			glBindBuffer(GL_ARRAY_BUFFER, vboHalfEdgeMesh[0]);
//			glBufferData(GL_ARRAY_BUFFER, halfEdgeMeshVertices.size() * sizeof(float) * 3, halfEdgeMeshVertices.data(), GL_STATIC_DRAW);
//
//			glBindBuffer(GL_ARRAY_BUFFER, vboHalfEdgeMesh[1]);
//			glBufferData(GL_ARRAY_BUFFER, halfEdgeMeshColors.size() * sizeof(float) * 3, halfEdgeMeshColors.data(), GL_STATIC_DRAW);
//		}
//
//	}
//}
//
///*********************************************
//Mouse selection on heModel types (iterator linked list)
//*********************************************/
//void mouseTriangleOperation() {
//
//	if (leftMouseClick) {
//
//			leftMouseClick = false;
//
//			float x = 2.0f * (float(leftMouseClickX) / float(WIDTH)) - 1.0f;
//			float y = 1.0f - 2.0f * (float(leftMouseClickY) / float(HEIGHT));
//
//			glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
//
//			glm::vec4 ray_eye = glm::inverse(projMatrix) * ray_clip;
//			ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
//
//			glm::vec3 ray_wor = glm::vec3((glm::inverse(viewMatrix) * ray_eye));
//			ray_wor = glm::normalize(ray_wor);
//
//			vector<pair<float, pair<int, FaceIter>>> list;
//			int i = 0;
//			for (FaceIter f = heMesh.facesBegin(); f != heMesh.facesEnd(); f++) {
//
//				glm::mat4 modelMatrixIntersect = glm::mat4(1.0f);
//				modelMatrixIntersect = glm::translate(modelMatrixIntersect, glm::vec3(-10.0f, 0.0f, 0.0f));
//				modelMatrixIntersect = glm::scale(modelMatrixIntersect, glm::vec3(3.0f));
//
//				HalfedgeIter h = f->halfedge();
//				glm::vec3 v0 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//				h = h->next();
//				glm::vec3 v1 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//				h = h->next();
//				glm::vec3 v2 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//				float u, v;
//				float t = rayIntersectsTriangle(cam.position, ray_wor, v0, v1, v2, u, v);
//
//
//				if (t != -1.0f) {
//					std::cout << "t: " << t << " u: " << u << " v: " << v << std::endl;
//					list.push_back(pair<float, pair<int, FaceIter>>(t, pair<int, FaceIter>(i, f)));
//				}
//
//				i++;
//			}
//
//			
//
//			//If we dont even hit a triangle stop here
//			if (!list.empty()) {
//
//				//Sort all triangles hit by ray
//				sort(list.begin(), list.end());
//
//				//Find the closest edge to the selected point
//				//We need the barycentric coordinates of the intersection ray/triangle ( we could get this smarter then recalculating)
//				FaceIter f = list[0].second.second;
//				glm::mat4 modelMatrixIntersect = glm::mat4(1.0f);
//				modelMatrixIntersect = glm::translate(modelMatrixIntersect, glm::vec3(-10.0f, 0.0f, 0.0f));
//				modelMatrixIntersect = glm::scale(modelMatrixIntersect, glm::vec3(3.0f));
//
//				HalfedgeIter h = f->halfedge();
//				glm::vec3 v0 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//				h = h->next();
//				glm::vec3 v1 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//				h = h->next();
//				glm::vec3 v2 = glm::vec3(modelMatrixIntersect * glm::vec4(h->vertex()->position, 1.0f));
//
//
//				float u = -1.0f, v = -1.0f;
//				float t = rayIntersectsTriangle(cam.position, ray_wor, v0, v1, v2, u, v);
//				float w = 1.0f - u - v;
//				EdgeIter eToSplit;
//
//				//the POBLEM here...
//				//NOT: iterator memorizes no state, it resets itself!
//				//WAS: "1-u-v" is a INTEGER 1 --> 1.0f
//				if (u <= v && u <= w) {
//					eToSplit = f->halfedge()->next()->next()->edge(); // v0 -> v1 -> v2
//					//f->halfedge()->next(); //back to v0 is NOT needed! Iterator memorizes no state.
//				}
//				else if (v <= u && v <= w) {
//					eToSplit = f->halfedge()->edge(); //v0
//				}
//				else if (w <= u && w <= v) {
//					eToSplit = f->halfedge()->next()->edge(); // v0 -> v1 
//					//f->halfedge()->next()->next(); //back to v2-> v0  is NOT needed! Iterator memorizes no state.
//
//				}
//
//
//				switch (m_operation) {
//				case SPLIT:
//					heMesh.splitEdge(eToSplit);
//					break;
//				case FLIP:
//					heMesh.flipEdge(eToSplit);
//					break;
//				case COLLAPSE:
//					heMesh.collapseEdge(eToSplit);
//					break;
//				}
//
//				halfEdgeMeshColors.erase(halfEdgeMeshColors.begin(), halfEdgeMeshColors.end());
//				halfEdgeMeshVertices.erase(halfEdgeMeshVertices.begin(), halfEdgeMeshVertices.end());
//
//				for (FaceIter f = heMesh.facesBegin(); f != heMesh.facesEnd(); f++) {
//					//1. Half Edge of current face
//					HalfedgeIter h = f->halfedge();
//					halfEdgeMeshVertices.push_back(glm::vec3(h->vertex()->position.x, h->vertex()->position.y, h->vertex()->position.z));
//					halfEdgeMeshColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
//
//					//2. Half Edge of current face
//					h = h->next();
//					halfEdgeMeshVertices.push_back(glm::vec3(h->vertex()->position.x, h->vertex()->position.y, h->vertex()->position.z));
//					halfEdgeMeshColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
//
//					//3. Half Edge of current face
//					h = h->next();
//					halfEdgeMeshVertices.push_back(glm::vec3(h->vertex()->position.x, h->vertex()->position.y, h->vertex()->position.z));
//					halfEdgeMeshColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
//				}
//
//				glGenBuffers(2, vboHalfEdgeMesh);
//				glBindBuffer(GL_ARRAY_BUFFER, vboHalfEdgeMesh[0]);
//				glBufferData(GL_ARRAY_BUFFER, halfEdgeMeshVertices.size() * sizeof(float) * 3, halfEdgeMeshVertices.data(), GL_STATIC_DRAW);
//
//				glBindBuffer(GL_ARRAY_BUFFER, vboHalfEdgeMesh[1]);
//				glBufferData(GL_ARRAY_BUFFER, halfEdgeMeshColors.size() * sizeof(float) * 3, halfEdgeMeshColors.data(), GL_STATIC_DRAW);
//		}
//
//	}
//
//}

/* *********************************************************************************************************
Display + Main
********************************************************************************************************* */
void display() {
	//Timer
	timer.update();
	//FPS-Counter
	frame++;
	timeCounter = glutGet(GLUT_ELAPSED_TIME);
	if (timeCounter - timebase > 1000) {
		sprintf_s(timeString, "FPS:%4.2f", frame*1000.0 / (timeCounter - timebase));
		timebase = timeCounter;
		frame = 0;
		glutSetWindowTitle(timeString);
	}

	//OpenGL Clears
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	
	sponzaStandardScene();
	TwDraw(); //Draw Tweak-Bar

	glutSwapBuffers();
	glutPostRedisplay();

}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);

	glutCreateWindow("Basic Framework");

	setupTweakBar();

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error : " << glewGetErrorString(err) << std::endl;
	}

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(onMouseMove);
	glutMouseFunc(onMouseDown);
	glutReshapeFunc(reshape);
	glutIdleFunc(onIdle);

	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	TwGLUTModifiersFunc(glutGetModifiers);

	initGL();

	init();

	glutMainLoop();

	TwTerminate();

	return 0;
}










