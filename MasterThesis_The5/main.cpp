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
#include "nanoflannHelper.h"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "PC_Octree.h"
#include "ObjToPcd.h"

//Octree
PC_Octree* octree = 0;

//Time
Timer timer;
int frame;
long timeCounter, timebase;
char timeString[50];

//Resolution (has to be changed in helper.h too)
glm::vec2 resolution = glm::vec2(1024, 768);

//Externals
//cameraSystem cam(1.0f, 1.0f, glm::vec3(20.95f, 20.95f, -0.6f));
//cameraSystem cam(1.0f, 1.0f, glm::vec3(2.44f, 1.41f, 0.008f));
cameraSystem cam(1.0f, 1.0f, glm::vec3(6.36f, 2.94f, 0.05f));

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
Shader simpleSplatShader;
Shader basicColorShader;
Shader pointShader;

//Skybox
Shader skyboxShader;

//Models
simpleModel *teaPot = 0;
solidSphere *sphere = 0;

coordinateSystem *coordSysstem = 0;
viewFrustrum * viewfrustrum = 0;

// tweak bar
TwBar *tweakBar;
bool wireFrameTeapot = false;
bool backfaceCull = false;
bool drawOctreeBox = false;
bool setViewFrustrum = false;
bool showFrustrumCull = false;
bool fillViewFrustrum = false;
glm::vec3 lightDir;

typedef enum { QUAD_SPLATS,  POINTS_GL} SPLAT_TYPE;
SPLAT_TYPE m_currenSplatDraw = QUAD_SPLATS;

/* *********************************************************************************************************
TweakBar
********************************************************************************************************* */
void setupTweakBar() {
	TwInit(TW_OPENGL_CORE, NULL);
	tweakBar = TwNewBar("Settings");

	TwAddSeparator(tweakBar, "Splat Draw", nullptr);
	TwEnumVal Splats[] = { { QUAD_SPLATS, "QUAD_SPLATS" },{ POINTS_GL, "POINTS_GL" } };
	TwType SplatsTwType = TwDefineEnum("MeshType", Splats, 2);
	TwAddVarRW(tweakBar, "Splats", SplatsTwType, &m_currenSplatDraw, NULL);

	TwAddSeparator(tweakBar, "Utility", nullptr);
	TwAddSeparator(tweakBar, "Wireframe", nullptr);
	TwAddVarRW(tweakBar, "Wireframe Teapot", TW_TYPE_BOOLCPP, &wireFrameTeapot, " label='Wireframe Teapot' ");
	TwAddVarRW(tweakBar, "Backface Cull", TW_TYPE_BOOLCPP, &backfaceCull, " label='Backface Cull' ");

	TwAddSeparator(tweakBar, "Octree", nullptr);
	TwAddVarRW(tweakBar, "Octree Box", TW_TYPE_BOOLCPP, &drawOctreeBox, " label='Octree Box' ");

	TwAddSeparator(tweakBar, "Set Viewfrustrum", nullptr);
	TwAddVarRW(tweakBar, "ViewFrustrum", TW_TYPE_BOOLCPP, &setViewFrustrum, " label='ViewFrustrum' ");
	TwAddVarRW(tweakBar, "Fill Frustrum", TW_TYPE_BOOLCPP, &fillViewFrustrum, " label='Fill Frustrum' ");
	TwAddVarRW(tweakBar, "Frustrum Cull", TW_TYPE_BOOLCPP, &showFrustrumCull, " label='Frustrum Cull' ");

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
	glDeleteBuffers(2, vbo);
	indices.clear();
	vertices.clear();
}


std::vector<glm::vec3> colorOctree = { glm::vec3(1.0f, 0.0f, 0.0f),
										glm::vec3(1.0f, 0.6f, 0.0f),
										glm::vec3(0.0f, 0.5f, 0.5f),
										glm::vec3(1.0f, 1.0f, 1.0f),
										glm::vec3(0.0f, 1.0f, 0.0f),
										glm::vec3(0.0f, 0.0f, 0.0f),
										glm::vec3(1.0f, 0.0f, 1.0f),
										glm::vec3(0.0f, 0.0f, 1.0f),
									};

std::vector<glm::mat4> modelMatrixOctree;

void init() {
	/*****************************************************************
	Error Messages :(
	*****************************************************************/
	std::cout << "ERROR main: We normals seem to be missplaced. GL_FRONT should be GL_BACK!!!" << std::endl;

	/*****************************************************************
	VTK-File
	*****************************************************************/
	//load_VTKfile();
	//upload_VTKfile();



	/*****************************************************************
	obj-Models
	*****************************************************************/
	//teaPot = new simpleModel("C:/Dev/Assets/Teapot/teapot.obj", true);
	//std::cout << "TeaPot model: " << teaPot->vertices.size() << " vertices" << std::endl;

	//for (int i = 0; i < teaPot->vertices.size(); i++) {
	//	teaPot->vertices[i] = 2.0f * teaPot->vertices[i];
	//}
	//objToPcd(teaPot->vertices, teaPot->normals);

	
	/*****************************************************************
	obj-Models
	*****************************************************************/
	std::vector<glm::vec3> bigVertices, bigNormals, bigColors;
	std::vector<float> bigRadii;
	/*************
	***CityFront
	**************/
	//loadBigFile(bigVertices, bigNormals, bigRadii, bigColors, "C:/Users/Kompie8/Documents/Visual Studio 2015/Projects/MasterThesis_The5/MasterThesis_The5/pointclouds/cityFrontRGB.big");
	//for (int i = 0; i < bigVertices.size(); i++) {
	//	bigVertices[i] -= bigVertices[bigVertices.size() - 1] - glm::vec3(0.0f, 15.0f, 0.0f);
	//	float halfPi = 1.5707f;
	//	bigVertices[i] = glm::mat3(1.0f, 0.0f, 0.0f, 0.0f, glm::cos(halfPi), -glm::sin(halfPi), 0.0f, glm::sin(halfPi), glm::cos(halfPi)) * bigVertices[i];
	//
	//	if (glm::dot(-bigVertices[i], bigNormals[i]) > 0) {
	//		bigNormals[i] = -bigNormals[i];
	//	}
	//
	//}
	//octree = new PC_Octree(bigVertices, bigNormals, bigRadii, 100);
	
	/*************
	***TeaPot
	**************/
	//loadBigFile(bigVertices, bigNormals, bigRadii, "C:/Users/Kompie8/Documents/Visual Studio 2015/Projects/MasterThesis_The5/MasterThesis_The5/pointclouds/bigTeapotVNA_100.big");
	//octree = new PC_Octree(bigVertices, bigNormals, bigRadii, 10);

	/*************
	***Sphere
	**************/
	sphere = new solidSphere(1.0f, 30, 30);
	std::vector<glm::vec3> sphereNormals;
	std::vector<float> radiiSphere(sphere->vertices.size(), 1.0f);
	for (int i = 0; i < sphere->vertices.size(); i++) {
		sphereNormals.push_back(sphere->vertices[i]);
		sphere->vertices[i] = 3.0f * sphere->vertices[i];
	}
	octree = new PC_Octree(sphere->vertices, sphereNormals, radiiSphere, 100);
	


	int counter = 0;
	for (int i = 0; i < 8; i++) {
		if (octree->root.bitMaskChildren[i] == 1) {
			modelMatrixOctree.push_back(glm::mat4(1.0f));
			octree->getAabbLeafUniforms(modelMatrixOctree[counter], octree->root.children[counter]);
			counter++;
		}
	}

	std::cout << "-> Main: modelMatrixOctree.size " << modelMatrixOctree.size() << std::endl;
	std::cout << "-> Main: octree->modelMatrixLowestLeaf.size() " << octree->modelMatrixLowestLeaf.size() << std::endl;

	/*****************************************************************
	Coordinate System
	*****************************************************************/
	coordSysstem = new coordinateSystem();
	coordSysstem->upload();

	/*****************************************************************
	View Frusturm
	*****************************************************************/
	viewfrustrum = new viewFrustrum(glm::mat4(1.0f), viewMatrix, projMatrix, 5, glm::vec3(cam.viewDir));
	viewfrustrum->upload();

	/*****************************************************************
	Skybox (Only for aesthetic reasons, can be deleted)
	*****************************************************************/
	skybox.createSkybox(negz, posz, posy, negy, negx, posx);
}


void loadShader(bool init) {
	basicShader = Shader("./shader/basic.vs.glsl", "./shader/basic.fs.glsl");
	basicColorShader = Shader("./shader/basicColor.vs.glsl", "./shader/basicColor.fs.glsl");
	modelLoaderShader = Shader("./shader/modelLoader.vs.glsl", "./shader/modelLoader.fs.glsl");
	skyboxShader = Shader("./shader/skybox.vs.glsl", "./shader/skybox.fs.glsl");
	simpleSplatShader = Shader("./shader/simpleSplat.vs.glsl", "./shader/simpleSplat.fs.glsl", "./shader/simpleSplat.gs.glsl");

	pointShader = Shader("./shader/point.vs.glsl", "./shader/point.fs.glsl");
}

/* *********************************************************************************************************
Scenes: Unit cube + Pointcloud
********************************************************************************************************* */
void sponzaStandardScene(){
	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

	/* ********************************************
	Draw Skybox (Disable Culling, else we loose skybox!)
	**********************************************/
	glDisable(GL_CULL_FACE);

	//skyboxShader.enable();
	//skyboxShader.uniform("projMatrix", projMatrix);
	//skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	//skybox.Draw(skyboxShader);
	//skyboxShader.disable();

	/* ********************************************
	Coordinate System
	**********************************************/
	basicColorShader.enable();
	modelMatrix = glm::scale(glm::vec3(1.0f));
	basicColorShader.uniform("modelMatrix", modelMatrix);
	basicColorShader.uniform("viewMatrix", viewMatrix);
	basicColorShader.uniform("projMatrix", projMatrix);
	coordSysstem->draw();
	basicColorShader.disable();

	/* ********************************************
	Octree
	**********************************************/
	if (drawOctreeBox) {
		basicShader.enable();

		
		basicShader.uniform("viewMatrix", viewMatrix);
		basicShader.uniform("projMatrix", projMatrix);

		//Draw Single Box
		//octree->getAabbUniforms(modelMatrix);
		//basicShader.uniform("modelMatrix", modelMatrix);
		//basicShader.uniform("col", glm::vec3(1.0f, 0.0f, 1.0f));
		//octree->drawBox();

		//Draw first level of boxes
		//for (int i = 0; i < modelMatrixOctree.size(); i++) {
		//	basicShader.uniform("modelMatrix", modelMatrixOctree[i]);
		//	basicShader.uniform("col", colorOctree[i]);
		//	octree->drawBox();
		//}

		//Draw finest level of boxes
		for (int i = 0; i < octree->modelMatrixLowestLeaf.size(); i++) {
			basicShader.uniform("modelMatrix", octree->modelMatrixLowestLeaf[i]);
			basicShader.uniform("col", octree->colorLowestLeaf[i]);
			octree->drawBox();
		}

		basicShader.disable();
	}

	/* ********************************************
	Simple Splat
	**********************************************/
	if (backfaceCull) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	else {
		glDisable(GL_CULL_FACE);
	}


	
	if (wireFrameTeapot) {
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}
	
	switch (m_currenSplatDraw) {
	case(QUAD_SPLATS):
		simpleSplatShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		simpleSplatShader.uniform("modelMatrix", modelMatrix);
		simpleSplatShader.uniform("viewMatrix", viewMatrix);
		simpleSplatShader.uniform("projMatrix", projMatrix);
		simpleSplatShader.uniform("col", glm::vec3(1.0f, 0.0f, 0.0f));
		octree->drawPointCloud();
		simpleSplatShader.disable();
		break;
	case(POINTS_GL):
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		pointShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		pointShader.uniform("modelMatrix", modelMatrix);
		pointShader.uniform("viewMatrix", viewMatrix);
		pointShader.uniform("projMatrix", projMatrix);
		pointShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));

		pointShader.uniform("nearPlane", 1.0f);
		pointShader.uniform("farPlane", 500.0f);
		pointShader.uniform("viewPoint", glm::vec3(cam.position));

		octree->drawPointCloud();
		pointShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);
		break;
	}
	if (wireFrameTeapot) {
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}
	

	/* ********************************************
	View Frustrum
	**********************************************/
	if (setViewFrustrum) {
		setViewFrustrum = false;
		viewfrustrum->change(glm::mat4(1.0f), viewMatrix, projMatrix);
		//viewfrustrum->frustrumToBoxes(glm::vec3(cam.viewDir));
		viewfrustrum->getPlaneNormal(false);

		if (fillViewFrustrum) {
			viewfrustrum->uploadQuad();
		}
		else {
			viewfrustrum->upload();
		}
	}

	basicColorShader.enable();
	modelMatrix = glm::scale(glm::vec3(1.0f));
	basicColorShader.uniform("modelMatrix", modelMatrix);
	basicColorShader.uniform("viewMatrix", viewMatrix);
	basicColorShader.uniform("projMatrix", projMatrix);

	if (fillViewFrustrum) {
		viewfrustrum->drawQuad();
	} 
	else{
		viewfrustrum->draw();
	}

	basicColorShader.disable();

	basicShader.enable();
	basicShader.uniform("viewMatrix", viewMatrix);
	basicShader.uniform("projMatrix", projMatrix);
	if (showFrustrumCull) {
		octree->initViewFrustrumCull(octree->root, *viewfrustrum);
		for (int i = 0; i < octree->modelMatrixLowestLeaf.size(); i++) {
			basicShader.uniform("modelMatrix", octree->modelMatrixLowestLeaf[i]);
			basicShader.uniform("col", octree->colorLowestLeaf[i]);
			octree->drawBox();
		}
	}
	basicShader.disable();

	
	

	///* ********************************************
	//VTK-Mesh
	//**********************************************/
	//basicShader.enable();
	//if (wireFrameTeapot) {
	//	glPolygonMode(GL_FRONT, GL_LINE);
	//	glPolygonMode(GL_BACK, GL_LINE);
	//}

	//glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

	//basicShader.uniform("modelMatrix", modelMatrix);
	//basicShader.uniform("viewMatrix", viewMatrix);
	//basicShader.uniform("projMatrix", projMatrix);

	//basicShader.uniform("col", glm::vec3(1.0f, 0.0f, 0.0f));

	////draw_VTKfile();
	////teaPot->draw();
	//teaPot->drawPoints();

	//if (wireFrameTeapot) {
	//	glPolygonMode(GL_FRONT, GL_FILL);
	//	glPolygonMode(GL_BACK, GL_FILL);
	//}

	//basicShader.disable();
}	

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

	glClearColor(0.3f, 0.3f, 0.3f, 1);
	
	sponzaStandardScene();
	TwDraw(); //Draw Tweak-Bar

	glutSwapBuffers();
	glutPostRedisplay();

}

int main(int argc, char** argv) {
	//std::vector<glm::vec3> test;
	//std::cout << test.max_size() << std::endl;

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

	delete coordSysstem;
	delete octree;
	delete viewfrustrum;
	//delete_VTKfile();


return 0;
}










