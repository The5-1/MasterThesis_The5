#define GLEW_STATIC //Using the static lib, so we need to enable it
#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/wglew.h>
#include <Ant/AntTweakBar.h>
#include <memory>
#include <algorithm>
#include <ctime>
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

//#include <glm/gtc/matrix_transform.hpp>
//glm::mat4 test = glm::frustum

//Octree
PC_Octree* octree = 0;

//Time
Timer timer;
int frame;
long timeCounter, timebase;
char timeString[50];

//Resolution (has to be changed in helper.h too)
glm::vec2 resolution = glm::vec2(1800.0f, 1000.0f);
//glm::vec2 resolution = glm::vec2(1024.0f, 768.0f);


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

//Textures
FilterKernel* filter = 0;

//Shaders
Shader basicShader;
Shader modelLoaderShader;
Shader simpleSplatShader;
Shader basicColorShader;
Shader pointShader;
//FBO Shader
Shader quadScreenSizedShader;
Shader standardMiniColorFboShader;
Shader standardMiniDepthFboShader;
//FBO Shader
Shader pointGbufferShader;
Shader pointDeferredShader;
Shader pointFuzzyShader;
Shader pointFuzzyFinalShader;
Shader shader_Splat_DepthWithEpsillon;
Shader shader_DrawOnscreenQuad;
Shader shader_Splat_Fuzzy;

//Filter
Shader gaussFilterShader;
Shader oneDimKernelShader;

//Skybox
Shader skyboxShader;

//Models
simpleModel *teaPot = 0;
solidSphere *sphere = 0;
simpleQuad * quad = 0;
coordinateSystem *coordSysstem = 0;
viewFrustrum * viewfrustrum = 0;

//Frame buffer object
FBO *fbo = 0;
FBO *fbo2 = 0;

// tweak bar
TwBar *tweakBar;
bool wireFrameTeapot = false;
bool backfaceCull = false;
bool drawOctreeBox = false;
bool setViewFrustrum = false;
bool showFrustrumCull = false;
bool fillViewFrustrum = false;
bool debugView = true;
bool useGaussFilter = false;
int filterPasses = 5;
glm::vec3 lightPos = glm::vec3(10.0, 10.0, 0.0);
float glPointSizeFloat = 15.0f;
float depthEpsilonOffset = 0.05f;
typedef enum { QUAD_SPLATS, POINTS_GL } SPLAT_TYPE; SPLAT_TYPE m_currenSplatDraw = POINTS_GL;
typedef enum { MODE_FUZZY_WORKS, MODE_VORONOI_WITHBORDERS, DEBUG, MODE_ADDITIVE, MODE_GIT_TRIANGLE, MODE_SHOW_KERNEL_TEXTURE, MODE_FUZZY_TEST} RENDER_TYPE; RENDER_TYPE m_currenRender = MODE_FUZZY_WORKS;
bool screenshot = false;

/* *********************************************************************************************************
Helper Function
********************************************************************************************************* */
void drawFBO(FBO *_fbo) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	//Color
	standardMiniColorFboShader.enable();
	_fbo->bindTexture(0);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.5f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 1.0f));
	quad->draw();
	_fbo->unbindTexture(0);
	standardMiniColorFboShader.disable();

	//Depth
	standardMiniDepthFboShader.enable();
	_fbo->bindDepth();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.5f, 0.5f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	_fbo->unbindDepth();
	standardMiniDepthFboShader.disable();

	//Normal
	standardMiniColorFboShader.enable();
	_fbo->bindTexture(1);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 0.5f));
	quad->draw();
	_fbo->unbindTexture(1);
	standardMiniColorFboShader.disable();

	//Position
	standardMiniColorFboShader.enable();
	_fbo->bindTexture(2);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.5f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 0.5f));
	quad->draw();
	_fbo->unbindTexture(2);
	standardMiniColorFboShader.disable();
}


/* *********************************************************************************************************
TweakBar
********************************************************************************************************* */
void setupTweakBar() {
	TwInit(TW_OPENGL_CORE, NULL);
	tweakBar = TwNewBar("Settings");

	TwAddSeparator(tweakBar, "Splat Draw", nullptr);
	TwAddVarRW(tweakBar, "Screenshot", TW_TYPE_BOOLCPP, &screenshot, " label='Screenshot' ");
	TwEnumVal Splats[] = { { QUAD_SPLATS, "QUAD_SPLATS" },{ POINTS_GL, "POINTS_GL" } };
	TwType SplatsTwType = TwDefineEnum("MeshType", Splats, 2);
	TwAddVarRW(tweakBar, "Splats", SplatsTwType, &m_currenSplatDraw, NULL);
	TwAddVarRW(tweakBar, "glPointSize", TW_TYPE_FLOAT, &glPointSizeFloat, " label='glPointSize' min=0.0 step=10.0 max=1000.0");
	TwAddVarRW(tweakBar, "depthEpsilonOffset", TW_TYPE_FLOAT, &depthEpsilonOffset, " label='depthEpsilonOffset' min=-10.0 step=0.001 max=10.0");
	TwAddVarRW(tweakBar, "Light Position", TW_TYPE_DIR3F, &lightPos, "label='Light Position'");

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

	TwAddSeparator(tweakBar, "Filter", nullptr);
	TwAddVarRW(tweakBar, "Use Gauss", TW_TYPE_BOOLCPP, &useGaussFilter, " label='Use Gauss' ");
	TwAddVarRW(tweakBar, "Passes", TW_TYPE_INT16, &filterPasses, " label='Passes' min=0 step=1 max=100");

	TwAddSeparator(tweakBar, "Debug Options", nullptr);
	TwEnumVal render[] = { {MODE_VORONOI_WITHBORDERS, "MODE_VORONOI_WITHBORDERS"}, {DEBUG, "DEBUG"}, {MODE_ADDITIVE, "MODE_ADDITIVE"}, { MODE_FUZZY_WORKS, "MODE_FUZZY_WORKS"}, {MODE_FUZZY_TEST ,"MODE_FUZZY_TEST"}, { MODE_GIT_TRIANGLE , "MODE_GIT_TRIANGLE"}, { MODE_SHOW_KERNEL_TEXTURE, "MODE_SHOW_KERNEL_TEXTURE"} };
	TwType renderTwType = TwDefineEnum("renderType", render, 7);
	TwAddVarRW(tweakBar, "render", renderTwType, &m_currenRender, NULL);
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

void delete_VTKfile() {
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
	Screen-Quad
	*****************************************************************/
	quad = new simpleQuad();
	quad->upload();

	/*****************************************************************
	Filter
	*****************************************************************/
	filter = new FilterKernel();

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
	//loadBigFile(bigVertices, bigNormals, bigRadii, "C:/Users/The5/Documents/Visual Studio 2015/Projects/MasterThesis_The5/MasterThesis_The5/pointclouds/bigTeapotVNA_100.big");
	//octree = new PC_Octree(bigVertices, bigNormals, bigRadii, 10);

	/*************
	***NanoSuit //loadPolyFile(bigVertices, bigNormals, bigRadii, bigColors, "C:/Dev/Assets/Nanosuit/nanosuit.ply");
	**************/
	//loadPolyFile(bigVertices, bigNormals, bigRadii, bigColors, "C:/Dev/Assets/Nanosuit/Nanosuit ply/nanosuit_butterfly2.ply");
	//octree = new PC_Octree(bigVertices, bigNormals, bigColors, bigRadii, 10);


	/*************
	***Bunny //loadPolyFile(bigVertices, bigNormals, bigRadii, bigColors, "C:/Dev/Assets/Nanosuit/nanosuit.ply");
	**************/
	//loadPolyFile(bigVertices, bigNormals, bigRadii, bigColors, "D:/Dev/Assets/Pointcloud/Bunny.ply");
	//loadPolyFile(bigVertices, bigNormals, bigRadii, bigColors, "D:/Dev/Assets/Kuh/Punktwolke/Kuh_Punktwolke_withNormals_bitSmalerl.ply");
	loadPolyFile(bigVertices, bigNormals, bigRadii, bigColors, "D:/Dev/Assets/Kuh/Punktwolke/Kuh_Punktwolke_withNormals.ply");


	for (int i = 0; i < bigVertices.size(); i++) {
		bigVertices[i] = bigVertices[i];
	}
	octree = new PC_Octree(bigVertices, bigNormals, bigColors, bigRadii, 10);

	/*************
	***Sphere
	**************/
	//sphere = new solidSphere(1.0f, 30, 30);
	//std::vector<glm::vec3> sphereNormals;
	//std::vector<float> radiiSphere(sphere->vertices.size(), 1.0f);
	//for (int i = 0; i < sphere->vertices.size(); i++) {
	//	sphereNormals.push_back(sphere->vertices[i]);
	//	sphere->vertices[i] = 3.0f * sphere->vertices[i];
	//}
	//octree = new PC_Octree(sphere->vertices, sphereNormals, radiiSphere, 100);



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

	/*****************************************************************
	FBO
	*****************************************************************/
	fbo = new FBO("Gbuffer", WIDTH, HEIGHT, FBO_GBUFFER_32BIT);
	gl_check_error("fbo");

	fbo2 = new FBO("Gbuffer", WIDTH, HEIGHT, FBO_GBUFFER_32BIT);
	gl_check_error("fbo2");

	/*****************************************************************
	Texture
	*****************************************************************/
}


void loadShader(bool init) {
	basicShader = Shader("./shader/basic.vs.glsl", "./shader/basic.fs.glsl");
	basicColorShader = Shader("./shader/basicColor.vs.glsl", "./shader/basicColor.fs.glsl");
	modelLoaderShader = Shader("./shader/modelLoader.vs.glsl", "./shader/modelLoader.fs.glsl");
	skyboxShader = Shader("./shader/skybox.vs.glsl", "./shader/skybox.fs.glsl");
	simpleSplatShader = Shader("./shader/simpleSplat.vs.glsl", "./shader/simpleSplat.fs.glsl", "./shader/simpleSplat.gs.glsl");
	pointShader = Shader("./shader/point.vs.glsl", "./shader/point.fs.glsl");

	//Deferred
	pointGbufferShader = Shader("./shader/PointGbuffer/pointGbuffer.vs.glsl", "./shader/PointGbuffer/pointGbuffer.fs.glsl");
	pointDeferredShader = Shader("./shader/PointGbuffer/pointDeferred.vs.glsl", "./shader/PointGbuffer/pointDeferred.fs.glsl");
	pointFuzzyShader = Shader("./shader/PointGbuffer/pointFuzzy.vs.glsl", "./shader/PointGbuffer/pointFuzzy.fs.glsl");
	pointFuzzyFinalShader = Shader("./shader/PointGbuffer/pointFuzzyFinal.vs.glsl", "./shader/PointGbuffer/pointFuzzyFinal.fs.glsl");
	
	//Updated
	shader_Splat_DepthWithEpsillon = Shader("./shader/PointGbuffer/pointGbufferUpdated.vs.glsl", "./shader/PointGbuffer/pointGbufferUpdated.fs.glsl");
	shader_Splat_Fuzzy = Shader("./shader/PointGbuffer/pointGbufferUpdated2ndPass.vs.glsl", "./shader/PointGbuffer/pointGbufferUpdated2ndPass.fs.glsl");

	shader_DrawOnscreenQuad = Shader("./shader/PointGbuffer/pointDeferredUpdated.vs.glsl", "./shader/PointGbuffer/pointDeferredUpdated.fs.glsl");


	//FBO
	quadScreenSizedShader = Shader("./shader/FboShader/quadScreenSized.vs.glsl", "./shader/FboShader/quadScreenSized.fs.glsl");
	standardMiniColorFboShader = Shader("./shader/FboShader/standardMiniColorFBO.vs.glsl", "./shader/FboShader/standardMiniColorFBO.fs.glsl");
	standardMiniDepthFboShader = Shader("./shader/FboShader/standardMiniDepthFBO.vs.glsl", "./shader/FboShader/standardMiniDepthFBO.fs.glsl");
	gaussFilterShader = Shader("./shader/Filter/gaussFilter.vs.glsl", "./shader/Filter/gaussFilter.fs.glsl");

	//Debug-Shaders
	oneDimKernelShader = Shader("./shader/Filter/oneDimKernel.vs.glsl", "./shader/Filter/oneDimKernel.fs.glsl");
}

/* *********************************************************************************************************
Scenes: Unit cube + Pointcloud
********************************************************************************************************* */
//void standardScene() {
//	gl_check_error("standardScene");
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glEnable(GL_DEPTH_TEST);
//	glClearColor(0.3f, 0.3f, 0.3f, 1);
//}

void render_Voronoi_withBorders() {
	//gl_check_error("standardScene");

	//Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

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
	//basicColorShader.enable();
	//modelMatrix = glm::scale(glm::vec3(1.0f));
	//basicColorShader.uniform("modelMatrix", modelMatrix);
	//basicColorShader.uniform("viewMatrix", viewMatrix);
	//basicColorShader.uniform("projMatrix", projMatrix);
	//coordSysstem->draw();
	//basicColorShader.disable();

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
		glCullFace(GL_BACK);
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

		glActiveTexture(GL_TEXTURE0);
		filter->Bind();
		simpleSplatShader.uniform("filter_kernel", 0);

		octree->drawPointCloud();
		//for (int i = 0; i < 15; i++) {
		//		modelMatrix = glm::translate(glm::vec3(float(i) * 2.0f, 0.0f, 0.0f));
		//		simpleSplatShader.uniform("modelMatrix", modelMatrix);
		//		octree->drawPointCloud();
		//}

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
		
		pointShader.uniform("glPointSize", glPointSizeFloat);
		pointShader.uniform("depthToPosTexture", false);

		octree->drawPointCloud();
		//for (int i = 0; i < 15; i++) {
		//	modelMatrix = glm::translate(glm::vec3(float(i) * 2.0f, 0.0f, 0.0f));
		//	pointShader.uniform("modelMatrix", modelMatrix);
		//	octree->drawPointCloud();
		//}

		pointShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);
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
	else {
		//viewfrustrum->draw();
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

void render_Debug() {
	//gl_check_error("render_Debug");
	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.3f, 0.3f, 0.3f, 1);

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
		//basicColorShader.enable();
		//modelMatrix = glm::scale(glm::vec3(1.0f));
		//basicColorShader.uniform("modelMatrix", modelMatrix);
		//basicColorShader.uniform("viewMatrix", viewMatrix);
		//basicColorShader.uniform("projMatrix", projMatrix);
		//coordSysstem->draw();
		//basicColorShader.disable();

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
			pointShader.uniform("lightPos", lightPos);
			pointShader.uniform("glPointSize", glPointSizeFloat);

			pointShader.uniform("depthToPosTexture", false);

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
		else {
			//viewfrustrum->draw();
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
	}
	fbo->Unbind();


	/* #### FBO End #### */
	//GaussFilter
	if (useGaussFilter) {
		for (int i = 0; i < filterPasses; i++) {
			fbo2->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo->bindTexture(0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(1.0f, 0.0f));
				quad->draw();

				fbo->unbindTexture(0);
				fbo->unbindTexture(1);
				fbo->unbindTexture(2);
				fbo->unbindDepth();
				glActiveTexture(GL_TEXTURE0);

				gaussFilterShader.disable();
			}
			fbo2->Unbind();

			fbo->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo2->bindTexture(0, 0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo2->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo2->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo2->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(0.0f, 1.0f));
				quad->draw();

				fbo2->unbindTexture(0);
				fbo2->unbindTexture(1);
				fbo2->unbindTexture(2);
				fbo2->unbindDepth();
				glActiveTexture(GL_TEXTURE0);
				gaussFilterShader.disable();
			}
			fbo->Unbind();
		}
	}

	//Render to screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	//Color
	standardMiniColorFboShader.enable();
	fbo->bindTexture(0);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.5f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 1.0f));
	quad->draw();
	fbo->unbindTexture(0);
	standardMiniColorFboShader.disable();

	//Depth
	standardMiniDepthFboShader.enable();
	fbo->bindDepth();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.5f, 0.5f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	fbo->unbindDepth();
	standardMiniDepthFboShader.disable();

	//Normal
	standardMiniColorFboShader.enable();
	fbo->bindTexture(1);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 0.5f));
	quad->draw();
	fbo->unbindTexture(1);
	standardMiniColorFboShader.disable();

	//Position
	standardMiniColorFboShader.enable();
	fbo->bindTexture(2);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.5f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 0.5f));
	quad->draw();
	fbo->unbindTexture(2);
	standardMiniColorFboShader.disable();
}

void render_GIT_triangles() {
	fbo->Bind();{
		/* ********************************************
		modelMatrix
		**********************************************/
		glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		//glEnable(GL_BLEND);
		//glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
	   

		pointFuzzyShader.enable();

		pointFuzzyShader.uniform("modelview_matrix", viewMatrix * modelMatrix);
		pointFuzzyShader.uniform("projection_matrix", projMatrix);
		pointFuzzyShader.uniform("projection_matrix_inv", glm::inverse(projMatrix));

		//Kernel
		glActiveTexture(GL_TEXTURE0);
		filter->Bind();
		pointFuzzyShader.uniform("filter_kernel", 0);

		//Placeholder
		pointFuzzyShader.uniform("viewport", glm::vec4(0.0f, 0.0f, resolution.x, resolution.y));
		/*splat_renderer.cpp (449):
		Vector4f frustum_plane[6];

		Matrix4f const& projection_matrix = m_camera.get_projection_matrix();
		for (unsigned int i(0); i < 6; ++i)
		{
			frustum_plane[i] = projection_matrix.row(3) + (-1.0f + 2.0f
				* static_cast<float>(i % 2)) * projection_matrix.row(i / 2);
		}

		for (unsigned int i(0); i < 6; ++i)
		{
			frustum_plane[i] = (1.0f / frustum_plane[i].block<3, 1>(
				0, 0).norm()) * frustum_plane[i];
		}
		*/
		glm::vec4 frustum_plane[6];
		pointFuzzyShader.uniform("frustum_plane", frustum_plane);
		pointFuzzyShader.uniform("material_color", glm::vec3(1.0, 0.0, 0.0));
		pointFuzzyShader.uniform("material_shininess", 8.0f);
		pointFuzzyShader.uniform("radius_scale", 1.0f);
		pointFuzzyShader.uniform("ewa_radius", 1.0f);
		pointFuzzyShader.uniform("epsilon", 5.0f * 1e-3f);

		octree->drawPointCloud();

		pointFuzzyShader.disable();
		filter->Unbind();

		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);
	}fbo->Unbind();

	if (true) {
		drawFBO(fbo);
	}
	else {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		pointFuzzyFinalShader.enable();

		pointFuzzyFinalShader.uniform("projection_matrix_inv", glm::inverse(projMatrix));
		pointFuzzyFinalShader.uniform("viewport", glm::vec4(0.0f, 0.0f, resolution.x, resolution.y));
		pointFuzzyFinalShader.uniform("material_shininess", 8.0f);


		fbo->bindTexture(0, 0);
		pointFuzzyFinalShader.uniform("color_texture", 0);
		fbo->bindTexture(1, 1);
		pointFuzzyFinalShader.uniform("normal_texture", 1);
		fbo->bindDepth(2);
		pointFuzzyFinalShader.uniform("depth_texture", 2);

		quad->draw();
		pointFuzzyFinalShader.disable();
	}
}

void render_Additive() {
	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));

	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //disable color rendering

		/* ********************************************
		Simple Splat
		**********************************************/
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);	

		pointGbufferShader.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		pointGbufferShader.uniform("modelMatrix", modelMatrix);
		pointGbufferShader.uniform("viewMatrix", viewMatrix);
		pointGbufferShader.uniform("projMatrix", projMatrix);
		pointGbufferShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));

		pointGbufferShader.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		pointGbufferShader.uniform("nearPlane", 1.0f);
		pointGbufferShader.uniform("farPlane", 500.0f);
		pointGbufferShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferShader.uniform("glPointSize", glPointSizeFloat);

		//std::cout << "cam pos: " << cam.position.x << " " << cam.position.y << " " << cam.position.z << " " << cam.position.w << std::endl;
		pointGbufferShader.uniform("cameraPos", glm::vec3(cam.position));

		octree->drawPointCloud();
		pointGbufferShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);
			

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); //disable color rendering
	}
	fbo->Unbind();

	
	fbo->Bind();
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		pointGbufferShader.enable();
		modelMatrix = glm::mat4(1.0f);
		pointGbufferShader.uniform("modelMatrix", modelMatrix);
		pointGbufferShader.uniform("viewMatrix", viewMatrix);
		pointGbufferShader.uniform("projMatrix", projMatrix);
		pointGbufferShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));

		pointGbufferShader.uniform("depthEpsilonOffset", 0.0f);

		pointGbufferShader.uniform("nearPlane", 1.0f);
		pointGbufferShader.uniform("farPlane", 500.0f);
		pointGbufferShader.uniform("viewPoint", glm::vec3(cam.position));
		pointGbufferShader.uniform("glPointSize", glPointSizeFloat);

		octree->drawPointCloud();
		pointGbufferShader.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		glDepthMask(GL_TRUE);

		glDisable(GL_BLEND);
	}
	fbo->Unbind();

	//drawFBO(fbo);

	//fbo2->Bind();
	//{
	//	glClear(GL_COLOR_BUFFER_BIT);
	//	glDisable(GL_DEPTH_TEST);
	//	glDisable(GL_BLEND);
	//	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	//	pointDeferredShader.enable();
	//	fbo->bindTexture(0, 0);
	//	pointDeferredShader.uniform("texColor", 0);
	//	fbo->bindTexture(1, 1);
	//	pointDeferredShader.uniform("texNormal", 1);
	//	fbo->bindTexture(2, 2);
	//	pointDeferredShader.uniform("texPosition", 2);
	//	fbo->bindDepth(3);
	//	pointDeferredShader.uniform("texDepth", 3);
	//	glActiveTexture(GL_TEXTURE4);
	//	filter->Bind();
	//	pointDeferredShader.uniform("filter_kernel", 4);
	//	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	//	pointDeferredShader.uniform("lightVecV", glm::vec3(lightPosView));
	//	quad->draw();
	//	pointDeferredShader.disable();
	//}
	//fbo2->Unbind();
	//drawFBO(fbo2);

	/* #### FBO End #### */
	//GaussFilter
	if (false) {
		for (int i = 0; i < filterPasses; i++) {
			fbo2->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo->bindTexture(0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(1.0f, 0.0f));
				quad->draw();

				fbo->unbindTexture(0);
				fbo->unbindTexture(1);
				fbo->unbindTexture(2);
				fbo->unbindDepth();
				glActiveTexture(GL_TEXTURE0);

				gaussFilterShader.disable();
			}
			fbo2->Unbind();

			fbo->Bind();
			{
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);
				glClearColor(0.3f, 0.3f, 0.3f, 1);
				gaussFilterShader.enable();

				glActiveTexture(GL_TEXTURE0);
				fbo2->bindTexture(0, 0);
				gaussFilterShader.uniform("texColor", 0);
				glActiveTexture(GL_TEXTURE1);
				fbo2->bindTexture(1, 1);
				gaussFilterShader.uniform("texNormal", 1);
				glActiveTexture(GL_TEXTURE2);
				fbo2->bindTexture(2, 2);
				gaussFilterShader.uniform("texPosition", 2);
				glActiveTexture(GL_TEXTURE3);
				fbo2->bindDepth(3);
				gaussFilterShader.uniform("texDepth", 3);

				gaussFilterShader.uniform("resolutionWIDTH", (float)resolution.x);
				gaussFilterShader.uniform("resolutionHEIGHT", (float)resolution.y);
				gaussFilterShader.uniform("radius", 1.0f);
				gaussFilterShader.uniform("dir", glm::vec2(0.0f, 1.0f));
				quad->draw();

				fbo2->unbindTexture(0);
				fbo2->unbindTexture(1);
				fbo2->unbindTexture(2);
				fbo2->unbindDepth();
				glActiveTexture(GL_TEXTURE0);
				gaussFilterShader.disable();
			}
			fbo->Unbind();
		}
	}

	//Deferred Shading (Use this to render directly to screen, else use the fbo to see debug)
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	pointDeferredShader.enable();
	glActiveTexture(GL_TEXTURE0);
	fbo->bindTexture(0, 0);
	pointDeferredShader.uniform("texColor", 0);
	glActiveTexture(GL_TEXTURE1);
	fbo->bindTexture(1, 1);
	pointDeferredShader.uniform("texNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	fbo->bindTexture(2, 2);
	pointDeferredShader.uniform("texPosition", 2);
	glActiveTexture(GL_TEXTURE3);
	fbo->bindDepth(3);
	pointDeferredShader.uniform("texDepth", 3);

	glActiveTexture(GL_TEXTURE4);
	filter->Bind();
	pointDeferredShader.uniform("filter_kernel", 4);

	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	pointDeferredShader.uniform("lightVecV", glm::vec3(lightPosView));
	quad->draw();
	pointDeferredShader.disable();

	//std::cout << "IMPORTANT: Because of the 2-pass-depthbuffer, we can only render if the distance epsilon is GREATER 0" << std::endl;
}

void render_fuzzy_works() {
	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));
	glm::vec4 clearColor = glm::vec4(0.0, 0.0f, 0.0f, 0.0f);

	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);


		//This VS: MVP, set pointsize based on perspective, manual backface cull with NoV
		//This FS: Discard after radius, Write depth+epsillon manually in all 4 Buffers (using parabolla depth), No DepthTest yet!
		// ---> DEPTH Test is ENABLED! we set "gl_FragDepth = newDepth;" manually, but this will still affect future Depth tests
		// ---> We do make use of automatic depth test after all! But with our custom Depths!

		shader_Splat_DepthWithEpsillon.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		shader_Splat_DepthWithEpsillon.uniform("modelMatrix", modelMatrix);
		shader_Splat_DepthWithEpsillon.uniform("viewMatrix", viewMatrix);
		shader_Splat_DepthWithEpsillon.uniform("projMatrix", projMatrix);
		shader_Splat_DepthWithEpsillon.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		shader_Splat_DepthWithEpsillon.uniform("viewPoint", glm::vec3(cam.position));
		shader_Splat_DepthWithEpsillon.uniform("glPointSize", glPointSizeFloat);
		shader_Splat_DepthWithEpsillon.uniform("cameraPos", glm::vec3(cam.position));
		shader_Splat_DepthWithEpsillon.uniform("renderPass", 0); //unused
		shader_Splat_DepthWithEpsillon.uniform("clearColor", clearColor); //unused

		octree->drawPointCloud();
		shader_Splat_DepthWithEpsillon.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	fbo->Unbind();

	//drawFBO(fbo);

	fbo2->Bind();
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

		
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		//glDisable(GL_BLEND);
		//glBlendFunc(GL_SOURCE0_ALPHA, GL_ONE);

		//This VS:	MVP, set pointsize based on perspective, manual backface cull with NoV (essentially = shader_Splat_DepthWithEpsillon!!!)
		//This FS:	Manual Depth Test
		//			Get old Depth + current depth (calculated)
		//			Do fuzzy Blending (ADDITIVE BLENDING)
		//			Output color (premultiplied blending alpha) (+ 4x depth unused)
		// ---> We MANUALLY Depth Test here! Against the previous Depth!

		shader_Splat_Fuzzy.enable();
		modelMatrix = glm::mat4(1.0f);
		shader_Splat_Fuzzy.uniform("modelMatrix", modelMatrix);
		shader_Splat_Fuzzy.uniform("viewMatrix", viewMatrix);
		shader_Splat_Fuzzy.uniform("projMatrix", projMatrix);
		shader_Splat_Fuzzy.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));
		shader_Splat_Fuzzy.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		shader_Splat_Fuzzy.uniform("width", resolution.x);
		shader_Splat_Fuzzy.uniform("height", resolution.y);

		shader_Splat_Fuzzy.uniform("nearPlane", 1.0f); //unused, we store in a linear color buffer instead of calculating the depth!
		shader_Splat_Fuzzy.uniform("farPlane", 500.0f); //unused, we store in a linear color buffer instead of calculating the depth!
		shader_Splat_Fuzzy.uniform("viewPoint", glm::vec3(cam.position));
		shader_Splat_Fuzzy.uniform("glPointSize", glPointSizeFloat);
		shader_Splat_Fuzzy.uniform("renderPass", 1); //unused
		shader_Splat_Fuzzy.uniform("clearColor", clearColor); //unused

		fbo->bindTexture(0, 0); //we use the linear color-buffer depth as input! No linearization of actual depth necessary!
		shader_Splat_Fuzzy.uniform("texDepth", 0);
		glActiveTexture(GL_TEXTURE1);
		filter->Bind();
		shader_Splat_Fuzzy.uniform("filter_kernel", 1); //this is no FILTER and no KERNEL, thats just a gauss gradient!

		octree->drawPointCloud();
		shader_Splat_Fuzzy.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		glDepthMask(GL_TRUE);

		glDisable(GL_BLEND);
	}
	fbo2->Unbind();

	//Deferred Shading (Use this to render directly to screen, else use the fbo to see debug)
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Alpha Blending
	glDisable(GL_BLEND);

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);


	//This VS:	Screen-Sited Quad
	//This FS:	Manual Depth Test

	shader_DrawOnscreenQuad.enable();
	glActiveTexture(GL_TEXTURE0);
	fbo2->bindTexture(0, 0);
	shader_DrawOnscreenQuad.uniform("texColor", 0);
	glActiveTexture(GL_TEXTURE1);
	fbo2->bindTexture(1, 1);
	shader_DrawOnscreenQuad.uniform("texNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	fbo2->bindTexture(2, 2);
	shader_DrawOnscreenQuad.uniform("texPosition", 2);
	glActiveTexture(GL_TEXTURE3);
	fbo2->bindDepth(3);
	shader_DrawOnscreenQuad.uniform("texDepth", 3);

	glActiveTexture(GL_TEXTURE4);
	filter->Bind();
	shader_DrawOnscreenQuad.uniform("filter_kernel", 4);

	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	shader_DrawOnscreenQuad.uniform("lightVecV", glm::vec3(lightPosView));
	quad->draw();
	shader_DrawOnscreenQuad.disable();


	//if (screenshot) {
	//	screenshot = false;
	//	/*
	//	https://www.gamedev.net/forums/topic/436385-how-to-save-a-screenshot-in-opengl/
	//	*/
	//	const int width = WIDTH;
	//	const int height = HEIGHT;

	//	const size_t bytesPerPixel = 3;	// RGB
	//	const size_t imageSizeInBytes = bytesPerPixel * size_t(width) * size_t(height);

	//	// Allocate with malloc, because the data will be managed by wxImage
	//	BYTE* pixels = static_cast<BYTE*>(malloc(imageSizeInBytes));

	//	// glReadPixels takes the lower-left corner, while GetViewportOffset gets the top left corner
	//	GLint m_viewport[4];
	//	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//	// glReadPixels can align the first pixel in each row at 1-, 2-, 4- and 8-byte boundaries. We
	//	// have allocated the exact size needed for the image so we have to use 1-byte alignment
	//	// (otherwise glReadPixels would write out of bounds)
	//	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	//	glReadPixels(m_viewport[0], m_viewport[1], width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels); //Only works on FBO's

	//																								 // glReadPixels reads the given rectangle from bottom-left to top-right, so we must
	//																								 // reverse it
	//	for (int y = 0; y < height / 2; y++)
	//	{
	//		const int swapY = height - y - 1;
	//		for (int x = 0; x < width; x++)
	//		{
	//			const int offset = int(bytesPerPixel) * (x + y * width);
	//			const int swapOffset = int(bytesPerPixel) * (x + swapY * width);

	//			// Swap R, G and B of the 2 pixels
	//			std::swap(pixels[offset + 0], pixels[swapOffset + 0]);
	//			std::swap(pixels[offset + 1], pixels[swapOffset + 1]);
	//			std::swap(pixels[offset + 2], pixels[swapOffset + 2]);
	//		}
	//	}
	//	stbi_write_png("screenshot.png", WIDTH, HEIGHT, 3, pixels, 0);
	//}
}

void render_fuzzy_test() {
	if (setViewFrustrum) {
	}
	else {
		viewfrustrum->change(glm::mat4(1.0f), viewMatrix, projMatrix);
		//viewfrustrum->frustrumToBoxes(glm::vec3(cam.viewDir));
		viewfrustrum->getPlaneNormal(false);
		viewfrustrum->upload();
	}

	/* ********************************************
	modelMatrix
	**********************************************/
	glm::mat4 modelMatrix = glm::scale(glm::vec3(1.0f));
	glm::vec4 clearColor = glm::vec4(0.0, 0.0f, 0.0f, 0.0f);

	/* #### FBO ####*/
	fbo->Bind();
	{
		//Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		//glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		shader_Splat_DepthWithEpsillon.enable();
		modelMatrix = glm::scale(glm::vec3(1.0f));
		shader_Splat_DepthWithEpsillon.uniform("modelMatrix", modelMatrix);
		shader_Splat_DepthWithEpsillon.uniform("viewMatrix", viewMatrix);
		shader_Splat_DepthWithEpsillon.uniform("projMatrix", projMatrix);
		shader_Splat_DepthWithEpsillon.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		shader_Splat_DepthWithEpsillon.uniform("viewPoint", glm::vec3(cam.position));
		shader_Splat_DepthWithEpsillon.uniform("glPointSize", glPointSizeFloat);
		shader_Splat_DepthWithEpsillon.uniform("cameraPos", glm::vec3(cam.position));
		shader_Splat_DepthWithEpsillon.uniform("renderPass", 0);
		shader_Splat_DepthWithEpsillon.uniform("clearColor", clearColor);

		octree->drawPointCloudInFrustrumPrep(octree->root, *viewfrustrum);

		shader_Splat_DepthWithEpsillon.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	fbo->Unbind();

	//drawFBO(fbo);

	fbo2->Bind();
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);


		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		//glDisable(GL_BLEND);
		//glBlendFunc(GL_SOURCE0_ALPHA, GL_ONE);

		shader_Splat_Fuzzy.enable();
		modelMatrix = glm::mat4(1.0f);
		shader_Splat_Fuzzy.uniform("modelMatrix", modelMatrix);
		shader_Splat_Fuzzy.uniform("viewMatrix", viewMatrix);
		shader_Splat_Fuzzy.uniform("projMatrix", projMatrix);
		shader_Splat_Fuzzy.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));
		shader_Splat_Fuzzy.uniform("depthEpsilonOffset", depthEpsilonOffset + 0.001f);

		shader_Splat_Fuzzy.uniform("width", resolution.x);
		shader_Splat_Fuzzy.uniform("height", resolution.y);

		shader_Splat_Fuzzy.uniform("nearPlane", 1.0f);
		shader_Splat_Fuzzy.uniform("farPlane", 500.0f);
		shader_Splat_Fuzzy.uniform("viewPoint", glm::vec3(cam.position));
		shader_Splat_Fuzzy.uniform("glPointSize", glPointSizeFloat);
		shader_Splat_Fuzzy.uniform("renderPass", 1);
		shader_Splat_Fuzzy.uniform("clearColor", clearColor);

		fbo->bindTexture(0, 0);
		shader_Splat_Fuzzy.uniform("texDepth", 0);
		glActiveTexture(GL_TEXTURE1);
		filter->Bind();
		shader_Splat_Fuzzy.uniform("filter_kernel", 1);

		octree->drawPointCloudInFrustrumPrep(octree->root, *viewfrustrum);
		shader_Splat_Fuzzy.disable();
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_PROGRAM_POINT_SIZE);

		glDepthMask(GL_TRUE);

		glDisable(GL_BLEND);
	}
	fbo2->Unbind();

	//Deferred Shading (Use this to render directly to screen, else use the fbo to see debug)
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Alpha Blending
	glDisable(GL_BLEND);

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	shader_DrawOnscreenQuad.enable();
	glActiveTexture(GL_TEXTURE0);
	fbo2->bindTexture(0, 0);
	shader_DrawOnscreenQuad.uniform("texColor", 0);
	glActiveTexture(GL_TEXTURE1);
	fbo2->bindTexture(1, 1);
	shader_DrawOnscreenQuad.uniform("texNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	fbo2->bindTexture(2, 2);
	shader_DrawOnscreenQuad.uniform("texPosition", 2);
	glActiveTexture(GL_TEXTURE3);
	fbo2->bindDepth(3);
	shader_DrawOnscreenQuad.uniform("texDepth", 3);

	glActiveTexture(GL_TEXTURE4);
	filter->Bind();
	shader_DrawOnscreenQuad.uniform("filter_kernel", 4);

	glm::vec4 lightPosView = viewMatrix * glm::vec4(lightPos, 0.0);
	shader_DrawOnscreenQuad.uniform("lightVecV", glm::vec3(lightPosView));
	quad->draw();
	shader_DrawOnscreenQuad.disable();
}

void render_KernelTexture(){
	//Debug Render Filter
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1);
	oneDimKernelShader.enable();
	glActiveTexture(GL_TEXTURE0);
	filter->Bind();
	oneDimKernelShader.uniform("filter_kernel", 0);
	quad->draw();
	filter->Unbind();
	oneDimKernelShader.disable();
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

	switch (m_currenRender) {
	case MODE_FUZZY_WORKS:
		render_fuzzy_works(); //Fuzzy splatting with blur filter to blend more smoothly (why does this currently even work!?)
		break;
	case MODE_FUZZY_TEST:
		render_fuzzy_test(); //Pure fuzzy splatting (WIP! currently broken!)
		break;
	case MODE_VORONOI_WITHBORDERS:
		render_Voronoi_withBorders(); //depth based voroni splats (with border for visualization)
		break;
	case DEBUG:
		render_Debug(); //show debug Buffers
		break;
	case MODE_ADDITIVE:
		render_Additive(); //failed experiment to blend with using just one FBO
		break;
	case MODE_GIT_TRIANGLE:
		render_GIT_triangles(); //some approach from git, not working
		break;
	case MODE_SHOW_KERNEL_TEXTURE:
		render_KernelTexture(); //preview texture
		break;
	};

	if (screenshot) {
		screenshot = false;
		/*
		https://www.gamedev.net/forums/topic/436385-how-to-save-a-screenshot-in-opengl/
		*/
		const int width = WIDTH;
		const int height = HEIGHT;

		const size_t bytesPerPixel = 3;	// RGB
		const size_t imageSizeInBytes = bytesPerPixel * size_t(width) * size_t(height);

		// Allocate with malloc, because the data will be managed by wxImage
		BYTE* pixels = static_cast<BYTE*>(malloc(imageSizeInBytes));

		// glReadPixels takes the lower-left corner, while GetViewportOffset gets the top left corner
		GLint m_viewport[4];
		glGetIntegerv(GL_VIEWPORT, m_viewport);

		// glReadPixels can align the first pixel in each row at 1-, 2-, 4- and 8-byte boundaries. We
		// have allocated the exact size needed for the image so we have to use 1-byte alignment
		// (otherwise glReadPixels would write out of bounds)
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(m_viewport[0], m_viewport[1], width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels); //Only works on FBO's

																									 // glReadPixels reads the given rectangle from bottom-left to top-right, so we must
																									 // reverse it
		for (int y = 0; y < height / 2; y++)
		{
			const int swapY = height - y - 1;
			for (int x = 0; x < width; x++)
			{
				const int offset = int(bytesPerPixel) * (x + y * width);
				const int swapOffset = int(bytesPerPixel) * (x + swapY * width);

				// Swap R, G and B of the 2 pixels
				std::swap(pixels[offset + 0], pixels[swapOffset + 0]);
				std::swap(pixels[offset + 1], pixels[swapOffset + 1]);
				std::swap(pixels[offset + 2], pixels[swapOffset + 2]);
			}
		}
		stbi_write_png("screenshot.png", WIDTH, HEIGHT, 3, pixels, 0);
	}

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

	//Disable Vsync, requires #include <GL/wglew.h>
	//wglSwapIntervalEXT(0);

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










