// MD2 animation renderer
// This demo will load and render an animated MD2 model, an OBJ model and a skybox
// Most of the OpenGL code for dealing with buffer objects, etc has been moved to a 
// utility library, to make creation and display of mesh objects as simple as possible

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stack>
#include <vector>
 
using namespace std;
using namespace glm;
#define DEG_TO_RADIAN 0.017453293

// Globals
// Real programs don't use globals :-D

int SCREENWIDTH = 600;
int SCREENHEIGHT = 400;

GLuint meshIndexCount = 0;
GLuint toonIndexCount = 0;

GLuint meshObjects[2];

GLuint skyboxProgram;
GLuint toonShaderProgram;
GLuint blurprogram[5];

GLfloat r = 0.0f;

glm::vec3 eye(0.0f, 1.0f, 0.0f);
glm::vec3 at(0.0f, 1.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

stack<glm::mat4> mvStack;

// TEXTURE STUFF
GLuint texture;
GLuint skybox;
GLuint fbo[2];
GLuint fbtex[2];

int mode = 0, iterations = 1;

rt3d::lightStruct light0 = {
	{ 0.3f, 0.3f, 0.3f, 1.0f }, // ambient
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // diffuse
	{ 1.0f, 1.0f, 1.0f, 1.0f }, // specular
	{ -10.0f, 10.0f, 10.0f, 1.0f }  // position
}; glm::vec4 lightPos(-3.0f, 3.0f, 3.0f, 1.0f); //light position


rt3d::materialStruct material0 = {
	{ 0.2f, 0.4f, 0.2f, 1.0f }, // ambient
	{ 0.5f, 1.0f, 0.5f, 1.0f }, // diffuse
	{ 0.0f, 0.1f, 0.0f, 1.0f }, // specular
	2.0f  // shininess
};

// attenuation for lighting
float attConstant = 1.0f;
float attLinear = 0.0f;
float attQuadratic = 0.0f;

//frame buffer atempt
 /*
void createFramebuffer(GLuint& fbo, GLuint &fbtex) {
	glGenTextures(1, &fbtex);
	glBindTexture(GL_TEXTURE_2D, fbtex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbtex, 0);
	{
		GLenum status;
		status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		switch (status) {
		case GL_FRAMEBUFFER_COMPLETE:
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			cerr << "Error: unsupported framebuffer format" << endl;
			exit(0);
		default:
			cerr << "Error: invalid framebuffer config" << endl;
			exit(0);
		}
	}
}*/

// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

													   // Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

// A simple texture loading function
// lots of room for improvement - and better error checking!


GLuint loadCubeMap(const char *fname[6], GLuint *texID)
{
	glGenTextures(1, texID); // generate texture ID
	GLenum sides[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y };
	SDL_Surface *tmpSurface;

	glBindTexture(GL_TEXTURE_CUBE_MAP, *texID); // bind texture and set parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i<6; i++)
	{
		// load file - using core SDL library
		tmpSurface = SDL_LoadBMP(fname[i]);
		if (!tmpSurface)
		{
			std::cout << "Error loading bitmap" << std::endl;
			return *texID;
		}

		glTexImage2D(sides[i], 0, GL_RGB, tmpSurface->w, tmpSurface->h, 0,
			GL_BGR, GL_UNSIGNED_BYTE, tmpSurface->pixels);
		// texture loaded, free the temporary buffer
		SDL_FreeSurface(tmpSurface);
	}
	return *texID;	// return value of texure ID, redundant really
}

void init(void) {
	const char *cubeTexFiles[6] = {
		"town-skybox/Town_bk.bmp", "town-skybox/Town_ft.bmp",
		"town-skybox/Town_rt.bmp", "town-skybox/Town_lf.bmp",
		"town-skybox/Town_up.bmp", "town-skybox/Town_dn.bmp"
	};
	loadCubeMap(cubeTexFiles, &skybox);



	skyboxProgram = rt3d::initShaders("textured.vert", "textured.frag");

	toonShaderProgram = rt3d::initShaders("toon.vert", "toon.frag");
	rt3d::setLight(toonShaderProgram, light0);
	rt3d::setMaterial(toonShaderProgram, material0);
	GLuint uniformIndex = glGetUniformLocation(toonShaderProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);

	//// create temporary framebuffer
	//createFramebuffer(fbo[0], fbtex[0]);
	//createFramebuffer(fbo[1], fbtex[1]);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

//	blur attempt

	//blurprogram[0] = rt3d::initShaders("passthrough.vs", "passthrough.fs");
	//blurprogram[1] = rt3d::initShaders("passthrough.vs", "discrete_vert.fs");
	//blurprogram[2] = rt3d::initShaders("passthrough.vs", "linear_vert.fs");
	//blurprogram[3] = rt3d::initShaders("passthrough.vs", "discrete_horiz.fs");
	//blurprogram[4] = rt3d::initShaders("passthrough.vs", "linear_horiz.fs");



	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;




	rt3d::loadObj("cube.obj", verts, norms, tex_coords, indices);
	GLuint size = indices.size();
	meshIndexCount = size;


	meshObjects[0] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), size, indices.data());

	verts.clear(); norms.clear(); tex_coords.clear(); indices.clear();

	rt3d::loadObj("bunny-5000.obj", verts, norms, tex_coords, indices);
	toonIndexCount = indices.size();
	meshObjects[1] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), nullptr, toonIndexCount, indices.data());

	//An attempt at a framebuffer to render the scene to a textureto work with the blur
	 
	 //The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	//fbo[0] = 0;
 //	glGenFramebuffers(1, &fbo[0]);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
 //
	//// The texture we're going to render to
 //	glGenTextures(1, &texture);

	//// "Bind" the newly created texture : all future texture functions will modify this texture
	//glBindTexture(GL_TEXTURE_2D, texture);

	//// Give an empty image to OpenGL ( the last "0" )
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	//// Poor filtering. Needed !
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 //
	// 
	// 

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
 

glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::sin(r*DEG_TO_RADIAN), pos.y, pos.z - d*std::cos(r*DEG_TO_RADIAN));

}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::cos(r*DEG_TO_RADIAN), pos.y, pos.z + d*std::sin(r*DEG_TO_RADIAN));
}

void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W]) eye = moveForward(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_S]) eye = moveForward(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_A]) eye = moveRight(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_D]) eye = moveRight(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_R]) eye.y += 0.1;
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1;

	if (keys[SDL_SCANCODE_COMMA]) r -= 1.0f;
	if (keys[SDL_SCANCODE_PERIOD]) r += 1.0f;

	glm::vec3 aux = glm::vec3(lightPos);
	if (keys[SDL_SCANCODE_UP]) aux = moveForward(aux, r, +0.1f);
	if (keys[SDL_SCANCODE_DOWN]) aux = moveForward(aux, r, -0.1f);
	if (keys[SDL_SCANCODE_LEFT]) aux = moveRight(aux, r, -0.1f);
	if (keys[SDL_SCANCODE_RIGHT]) aux = moveRight(aux, r, 0.1f);
	if (keys[SDL_SCANCODE_O]) aux.y += 0.1f;
	if (keys[SDL_SCANCODE_L]) aux.y -= 0.1f;

	lightPos = glm::vec4(aux, lightPos.w);


	//blurring attempt

	//if (keys[SDL_SCANCODE_SPACE]) mode = ++mode % 3;

	//if (keys[SDL_SCANCODE_1]) iterations = 1;
	//if (keys[SDL_SCANCODE_2]) iterations = 2;
	//if (keys[SDL_SCANCODE_3]) iterations = 3;
	//if (keys[SDL_SCANCODE_4]) iterations = 4;
	//if (keys[SDL_SCANCODE_5]) iterations = 5;
	//if (keys[SDL_SCANCODE_6]) iterations = 6;
	//if (keys[SDL_SCANCODE_7]) iterations = 7;
	//if (keys[SDL_SCANCODE_8]) iterations = 8;
	//if (keys[SDL_SCANCODE_9]) iterations = 9;
	
}

void draw(SDL_Window * window) {
	// clear the screen
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);


	glm::mat4 modelview(1.0); // set base position for scene
	mvStack.push(modelview);

	at = moveForward(eye, r, 1.0f);
	mvStack.top() = glm::lookAt(eye, at, up);

	// set up camera/eye position 
	// then render skybox as single cube using cube map
	glUseProgram(skyboxProgram);
	rt3d::setUniformMatrix4fv(skyboxProgram, "projection", glm::value_ptr(projection));
	glDepthMask(GL_FALSE); // make sure writing to update depth test is off
	glm::mat3 mvRotOnlyMat3 = glm::mat3(mvStack.top());
	mvStack.push(glm::mat4(mvRotOnlyMat3));

	glCullFace(GL_FRONT); // drawing inside of cube!
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.5f, 1.5f, 1.5f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();
	glCullFace(GL_BACK);
	glDepthMask(GL_TRUE);

	glm::vec4 tmp = mvStack.top()*lightPos;

	// draw a toon cube
	glUseProgram(toonShaderProgram);
	rt3d::setLightPos(toonShaderProgram, glm::value_ptr(tmp));
	rt3d::setUniformMatrix4fv(toonShaderProgram, "projection", glm::value_ptr(projection));
	glBindTexture(GL_TEXTURE_2D, texture);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, 2.2f, -8.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.0f, 1.1f, 1.0f));
	rt3d::setUniformMatrix4fv(toonShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(toonShaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[0], toonIndexCount, GL_TRIANGLES);
	mvStack.pop();

	//toon bunny
	glUseProgram(toonShaderProgram);
	rt3d::setLightPos(toonShaderProgram, glm::value_ptr(tmp));
	rt3d::setUniformMatrix4fv(toonShaderProgram, "projection", glm::value_ptr(projection));
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-3.0f, 0.2f, -8.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0, 20.0, 20.0));
	rt3d::setUniformMatrix4fv(toonShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(toonShaderProgram, material0);
	rt3d::drawIndexedMesh(meshObjects[1], toonIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// remember to use at least one pop operation per push...
	mvStack.pop(); // initial matrix
	glDepthMask(GL_TRUE);
	 

	//blurring attempt

	//glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);

	//ping pong between fbos
	//glBindTexture(GL_TEXTURE_2D, texture);

	//if (mode != 0) {
	//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[0]);

	//	glUseProgram(blurprogram[mode + 2]);
	//	glDrawArrays(GL_TRIANGLES, 0, 6);

	//	for (int i = 1; i < iterations; i++) {
	//		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
	//		glBindTexture(GL_TEXTURE_2D, fbtex[0]);

	//		glUseProgram(blurprogram[mode]);
	//		glDrawArrays(GL_TRIANGLES, 0, 6);

	//		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[0]);
	//	 	glBindTexture(GL_TEXTURE_2D, fbtex[1]);

	//		glUseProgram(blurprogram[mode + 2]);
	//		glDrawArrays(GL_TRIANGLES, 0, 6);
	//	}

	//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//	glBindTexture(GL_TEXTURE_2D, fbtex[0]);
	//}
	//glUseProgram(blurprogram[mode]);
	//glDrawArrays(GL_TRIANGLES, 0, 6);

	////full screen quad
	//GLuint vbo;
	//glGenBuffers(1, &vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//GLfloat quad[] = { 1.f, 1.f, -1.f, 1.f, -1.f,-1.f,
	//	-1.f,-1.f,  1.f,-1.f,  1.f, 1.f };
	//glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 6, quad, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)0);
	//glUseProgram(blurprogram[0]);


	SDL_GL_SwapWindow(window); // swap buffers
}



// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

								  // Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}