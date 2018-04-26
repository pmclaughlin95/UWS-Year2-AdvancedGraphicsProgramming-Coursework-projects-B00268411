#pragma once
#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include <SDL_ttf.h>
#include "boundingBox.h"


using namespace std;
#define DEG_TO_RADIAN 0.017453293

class levelDesign {
public:
	void init(void);
	void draw(SDL_Window * window);
	void update(void);

	GLuint textToTexture(const char * str, GLuint textID);
	GLuint loadBitmap(char *fname);
	GLuint loadCubeMap(const char *fname[6], GLuint *texID);
	glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d);
	glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d);
	//function for normal mappin tangents
	void calculateTangents(vector<GLfloat> &tangents, vector<GLfloat> &verts, vector<GLfloat> &normals, vector<GLfloat> &tex_coords, vector<GLuint> &indices);
	
	

protected:



	GLuint meshIndexCount = 0;
	GLuint toonIndexCount = 0;
	GLuint md2VertCount = 0;
	GLuint meshObjects[3];

	GLuint textureProgram;
	GLuint shaderProgram;
	GLuint skyboxProgram;
	GLuint toonShaderProgram;
	GLuint normalMapProgram;

	boundingBox* box[4];

	//globals for the blurring
	GLuint pboTex;
	GLuint pboTexSize;
	GLuint pboBuffer;
	GLuint screenHeight = 600;
	GLuint screenWidth = 800;
	

	GLuint uniformIndex;
	GLfloat r;

	glm::vec3 eye;
	glm::vec3 at;
	glm::vec3 up;

	stack<glm::mat4> mvStack;

	// TEXTURE STUFF
	GLuint textures[3];
	GLuint skybox[5];
	GLuint labels[5];

	rt3d::lightStruct light0 = {
		{ 0.01f, 0.01f, 0.01f, 1.0f }, // ambient
		{ 0.3f, 0.3f, 0.3f, 0.3f }, // diffuse
		{ 0.3f, 0.3f, 0.3f, 0.3f }, // specular
		{ -5.0f, 10.0f, 2.0f, 1.0f }  // position
	};
	glm::vec4 lightPos;

	//green
	rt3d::materialStruct material0 = {
		{ 0.2f, 0.4f, 0.2f, 1.0f }, // ambient
		{ 0.5f, 1.0f, 0.5f, 1.0f }, // diffuse
		{ 0.0f, 0.1f, 0.0f, 1.0f }, // specular
		2.0f  // shininess
	};
	
	//grey
	rt3d::materialStruct material1 = {
		{ 0.4f, 0.4f, 1.0f, 1.0f }, // ambient
		{ 0.8f, 0.8f, 1.0f, 1.0f }, // diffuse
		{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
		1.0f  // shininess
	};

	//gold
	rt3d::materialStruct materialGold = {
		{ 0.24725f, 0.1995f, 0.0745f, 1.0f },
		{ 0.75164f, 0.60648f, 0.22648f, 1.0f },
		{ 0.628281f, 0.555802f, 0.366065f, 1.0f },
		10  // shininess
	};

	// light attenuation
	float attConstant = 1.0f;
	float attLinear = 0.0f;
	float attQuadratic = 0.0f;


	float theta = 0.0f;

 

	TTF_Font * textFont;


};