#include "levelDesign.h"

// textToTexture
GLuint levelDesign::textToTexture(const char * str, GLuint textID) {
	TTF_Font *font = textFont;
	SDL_Color colour = { 255, 255, 255 };
	SDL_Color bg = { 0, 0, 0 };

	SDL_Surface *stringImage;
	stringImage = TTF_RenderText_Blended(font, str, colour);

	if (stringImage == NULL)
		//exitFatalError("String surface not created.");
		std::cout << "String surface not created." << std::endl;

	GLuint w = stringImage->w;
	GLuint h = stringImage->h;
	GLuint colours = stringImage->format->BytesPerPixel;

	GLuint format, internalFormat;
	if (colours == 4) {   // alpha
		if (stringImage->format->Rmask == 0x000000ff)
			format = GL_RGBA;
		else
			format = GL_BGRA;
	}
	else {             // no alpha
		if (stringImage->format->Rmask == 0x000000ff)
			format = GL_RGB;
		else
			format = GL_BGR;
	}
	internalFormat = (colours == 4) ? GL_RGBA : GL_RGB;

	GLuint texture = textID;

	if (texture == 0) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} 

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, stringImage->w, stringImage->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, stringImage->pixels);
	glBindTexture(GL_TEXTURE_2D, NULL);

	SDL_FreeSurface(stringImage);
	return texture;
}


// A simple cubemap loading function
GLuint levelDesign::loadCubeMap(const char *fname[6], GLuint *texID)
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

	GLuint externalFormat;
	for (int i = 0;i<6;i++)
	{
		// load file - using core SDL library
		tmpSurface = SDL_LoadBMP(fname[i]);
		if (!tmpSurface)
		{
			std::cout << "Error loading bitmap" << std::endl;
			return *texID;
		}

		SDL_PixelFormat *format = tmpSurface->format;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;

		glTexImage2D(sides[i], 0, GL_RGB, tmpSurface->w, tmpSurface->h, 0,
			externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
		// texture loaded, free the temporary buffer
		SDL_FreeSurface(tmpSurface);
	}
	return *texID;	// return value of texure ID, redundant really
}


//texture loading function
GLuint levelDesign::loadBitmap(char *fname) {
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID

							  // load file 
	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	SDL_PixelFormat *format = tmpSurface->format;

	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}

void levelDesign::calculateTangents(vector<GLfloat> &tangents, vector<GLfloat> &verts, vector<GLfloat> &normals, vector<GLfloat> &tex_coords, vector<GLuint> &indices) {

	// Code taken from http://www.terathon.com/code/tangent.html and modified slightly to use vectors instead of arrays
	// Lengyel, Eric. “Computing Tangent Space Basis Vectors for an Arbitrary Mesh”. Terathon Software 3D Graphics Library, 2001. 


	vector<glm::vec3> tan1(verts.size() / 3, glm::vec3(0.0f));
	vector<glm::vec3> tan2(verts.size() / 3, glm::vec3(0.0f));
	int triCount = indices.size() / 3;
	for (int c = 0; c < indices.size(); c += 3)
	{
		int i1 = indices[c];
		int i2 = indices[c + 1];
		int i3 = indices[c + 2];

		glm::vec3 v1(verts[i1 * 3], verts[i1 * 3 + 1], verts[i1 * 3 + 2]);
		glm::vec3 v2(verts[i2 * 3], verts[i2 * 3 + 1], verts[i2 * 3 + 2]);
		glm::vec3 v3(verts[i3 * 3], verts[i3 * 3 + 1], verts[i3 * 3 + 2]);

		glm::vec2 w1(tex_coords[i1 * 2], tex_coords[i1 * 2 + 1]);
		glm::vec2 w2(tex_coords[i2 * 2], tex_coords[i2 * 2 + 1]);
		glm::vec2 w3(tex_coords[i3 * 2], tex_coords[i3 * 2 + 1]);

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (int a = 0; a < verts.size(); a += 3)
	{
		glm::vec3 n(normals[a], normals[a + 1], normals[a + 2]);
		glm::vec3 t = tan1[a / 3];

		glm::vec3 tangent;
		tangent = (t - n * glm::normalize(glm::dot(n, t)));

		
		GLfloat w = (glm::dot(glm::cross(n, t), tan2[a / 3]) < 0.0f) ? -1.0f : 1.0f;

		tangents.push_back(tangent.x);
		tangents.push_back(tangent.y);
		tangents.push_back(tangent.z);
		tangents.push_back(w);

	}




}


void levelDesign::init(void) {
	
	eye = glm::vec3(-2.0f, 1.0f, 8.0f);
	at = glm::vec3(0.0f, 1.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	r = 0.0f;
	

	lightPos = glm::vec4(5.0f, 20.0f, 2.0f, 1.0f); //light position


	shaderProgram = rt3d::initShaders("phong-tex.vert", "phong-tex.frag");
	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, material0);
	// set light attenuation shader uniforms
	uniformIndex = glGetUniformLocation(shaderProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(shaderProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(shaderProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);



	toonShaderProgram = rt3d::initShaders("toon.vert", "toon.frag");
	rt3d::setLight(toonShaderProgram, light0);
	rt3d::setMaterial(toonShaderProgram, material0);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(toonShaderProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);

	

	textureProgram = rt3d::initShaders("textured.vert", "textured.frag");
	skyboxProgram = rt3d::initShaders("cubeMap.vert", "cubeMap.frag");



	const char *cubeTexFiles[6] = {
		"Ame_nebula/purplenebula_bk.bmp", "Ame_nebula/purplenebula_dn.bmp", "Ame_nebula/purplenebula_ft.bmp", "Ame_nebula/purplenebula_lf.bmp", "Ame_nebula/purplenebula_rt.bmp", "Ame_nebula/purplenebula_up.bmp"
	};
	loadCubeMap(cubeTexFiles, &skybox[0]);


	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;
	rt3d::loadObj("cube.obj", verts, norms, tex_coords, indices);
	meshIndexCount = indices.size();
	
//VBO for the bitangents
	vector<GLfloat> tangents;
	calculateTangents(tangents, verts, norms, tex_coords, indices);

	meshObjects[0] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), meshIndexCount, indices.data());


	glBindVertexArray(meshObjects[0]);
	GLuint VBO;
	glGenBuffers(1, &VBO);
	// VBO for tangent data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(GLfloat), tangents.data(), GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)5, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(5);
	glBindVertexArray(0);

//shader for normal mapping
	normalMapProgram = rt3d::initShaders("normalmap.vert", "normalmap.frag");
	glUseProgram(normalMapProgram);
	rt3d::setLight(normalMapProgram, light0);
	rt3d::setMaterial(normalMapProgram, material1);
	uniformIndex = glGetUniformLocation(normalMapProgram, "attConst");
	glUniform1f(uniformIndex, attConstant);
	uniformIndex = glGetUniformLocation(normalMapProgram, "attLinear");
	glUniform1f(uniformIndex, attLinear);
	uniformIndex = glGetUniformLocation(normalMapProgram, "attQuadratic");
	glUniform1f(uniformIndex, attQuadratic);
	// Binding tex handles to tex units to samplers under programmer control
	// set cubemap sampler to texture unit 1, arbitrarily
	uniformIndex = glGetUniformLocation(normalMapProgram, "normalMap");
	glUniform1i(uniformIndex, 1);
	// set tex sampler to texture unit 0, arbitrarily
	uniformIndex = glGetUniformLocation(normalMapProgram, "texMap");
	glUniform1i(uniformIndex, 0);




	textures[0] = loadBitmap("fabric.bmp");
	textures[2] = loadBitmap("studdedmetal.bmp");
	textures[3] = loadBitmap("metal-normalmap.bmp");
	textures[4] = loadBitmap("stoneground.bmp");
	textures[5] = loadBitmap("debris.bmp");

	verts.clear(); norms.clear();tex_coords.clear();indices.clear();
	rt3d::loadObj("bunny-5000.obj", verts, norms, tex_coords, indices);
	toonIndexCount = indices.size();
	meshObjects[2] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), nullptr, toonIndexCount, indices.data());


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// set up TrueType / SDL_ttf
	if (TTF_Init() == -1)
		cout << "TTF failed to initialise." << endl;

	textFont = TTF_OpenFont("MavenPro-Regular.ttf", 48);
	if (textFont == NULL)
		cout << "Failed to open font." << endl;

	labels[0] = 0;
	labels[0] = textToTexture(" Score: ", labels[0]);//set up of label. If dynamic, this should go in draw function


// Generate a texture for PBO operations
	glGenTextures(1, &pboTex);
	GLuint pboTexSize = screenWidth * screenHeight * 3 *
		sizeof(GLubyte);
	void* pboData = new GLubyte[pboTexSize];
	memset(pboData, 0, pboTexSize);	
// Set up the texture for PBO effects
	glBindTexture(GL_TEXTURE_2D, pboTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight,
		0, GL_RGB, GL_UNSIGNED_BYTE, pboData);

	// PBO itself
	glGenBuffers(1, &pboBuffer);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pboBuffer);
	glBufferData(GL_PIXEL_PACK_BUFFER, pboTexSize, pboData, GL_DYNAMIC_COPY);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	delete pboData;

}

glm::vec3 levelDesign::moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::sin(r*DEG_TO_RADIAN), pos.y, pos.z - d*std::cos(r*DEG_TO_RADIAN));
}

glm::vec3 levelDesign::moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::cos(r*DEG_TO_RADIAN), pos.y, pos.z + d*std::sin(r*DEG_TO_RADIAN));
}





void levelDesign::update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W]) eye = moveForward(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_S]) eye = moveForward(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_A]) eye = moveRight(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_D]) eye = moveRight(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_R]) eye.y += 0.1;
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1;

	if ( keys[SDL_SCANCODE_COMMA] ) r -= 2.0f;
	if ( keys[SDL_SCANCODE_PERIOD] ) r += 2.0f;
	

}

void levelDesign::draw(SDL_Window * window) {

		// clear the screen
		glEnable(GL_CULL_FACE);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection(1.0);
		projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);


		GLfloat scale(1.0f); // allow easy scaling of complete scene

		glm::mat4 modelview(1.0); // set base position for scene
		mvStack.push(modelview);

		at = moveForward(eye, r, 1.0f);
		mvStack.top() = glm::lookAt(eye, at, up);

		// skybox as single cube using cube map
		glUseProgram(skyboxProgram);
		rt3d::setUniformMatrix4fv(skyboxProgram, "projection", glm::value_ptr(projection));

		glDepthMask(GL_FALSE); 
		glm::mat3 mvRotOnlyMat3 = glm::mat3(mvStack.top());
		mvStack.push(glm::mat4(mvRotOnlyMat3));

		glCullFace(GL_FRONT); // drawing inside of cube
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox[0]);
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.5f, 1.5f, 1.5f));
		rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();
		glCullFace(GL_BACK); // drawing inside of cube


		glDepthMask(GL_TRUE); 


		


		glUseProgram(shaderProgram);
		rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));

		glm::vec4 tmp = mvStack.top()*lightPos;
		light0.position[0] = tmp.x;
		light0.position[1] = tmp.y;
		light0.position[2] = tmp.z;
		rt3d::setLightPos(shaderProgram, glm::value_ptr(tmp));

         //
		//Drawing of the level design starts here. This includes all ground planes and walls
	   //


		// room 1 ground plane
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.1f, -10.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		// room 2 ground plane
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(30.0f, -0.1f, 15.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		// secret corridor ground plane
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(40.0f, -0.1f, -15.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(10.0f, 0.1f, 2.5f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		// secret corridor ground plane
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(47.5f, -0.1f, -10.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(2.5f, 0.1f, 5.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//Normal Mapping!
		glUseProgram(normalMapProgram);
		rt3d::setUniformMatrix4fv(normalMapProgram, "projection", glm::value_ptr(projection));
		
		// Now bind textures to texture units
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[3]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);


		// first room horizontal wall front
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		glm::mat4 modelMatrix(1.0);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.01f, 10.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();


		//first room horizontal wall back
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.01f, -30.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();


		//first room vertical wall right
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(10.0f, -0.01f, -18.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 13.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//first room vertical wall left
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-30.0f, -0.01f, -10.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 20.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//second room vertical wall left
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(10.0f, -0.01f, 23.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 13.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//second room vertical wall right
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(50.0f, -0.01f, 15.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 20.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//second room horizontal wall front
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(27.5f, -0.01f, -5.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(17.5f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//second room horizontal wall back
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(30.0f, -0.01f, 35.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();


		//maze wall room 1 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(5.0f, -0.01f, -5.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 15.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 1 h
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.01f, -0.5f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(15.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 1 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-25.0f, -0.01f, -15.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 10.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 1 h
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-15.0f, -0.01f, -5.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(10.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 1 h
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.01f, -25.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(15.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 1 h
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.01f, -20.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(15.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 1 h
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.01f, -12.5f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(10.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 1 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, -0.01f, -12.5f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 7.5f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		
		
		//secret corridor wall
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(50.0f, -0.01f, -7.5f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 10.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//secret corridor wall
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(45.0f, -0.01f, -8.75f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 3.75f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//secret corridor wall
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(30.0f, -0.01f, -15.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 2.5f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//secret corridor wall
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(40.0f, -0.01f, -17.5f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(10.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//secret corridor wall
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(37.5f, -0.01f, -12.5f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(7.5f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();


		//maze wall room 2 h
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(30.0f, -0.01f, 0.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 2 h
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(32.5f, -0.01f, 10.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(7.5f, -5.0f, 0.00f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 2 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(15.0f, -0.01f, 15.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 15.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 2 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(20.0f, -0.01f, 15.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 15.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 2 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(25.0f, -0.01f, 20.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 10.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 2 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(32.5f, -0.01f, 20.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 10.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//maze wall room 2 v
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(40.0f, -0.01f, 20.0f));
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.0f, 5.0f, 15.0f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		 //
		//All other objects drawn from here on, including collectibles and end point.
	   //

		// draw collectible 1 
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-2.0f, 1.5f, -16.5f));
		mvStack.top() = glm::rotate(mvStack.top(), float(theta*DEG_TO_RADIAN), glm::vec3(1.0f, 1.0f, 1.0f));
		theta += 1.0f;
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.75f, 0.75f, 0.75f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, materialGold);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		// draw collectible 2 
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(35.0f, 1.5f, -15.0f));
		mvStack.top() = glm::rotate(mvStack.top(), float(theta*DEG_TO_RADIAN), glm::vec3(1.0f, 1.0f, 1.0f));
		theta += 1.0f;
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.75f, 0.75f, 0.75f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, materialGold);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		// draw collectible 3 	
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(36.0f, 1.5f, 15.0f));
		mvStack.top() = glm::rotate(mvStack.top(), float(theta*DEG_TO_RADIAN), glm::vec3(1.0f, 1.0f, 1.0f));
		theta += 1.0f;
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.75f, 0.75f, 0.75f));
		rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(shaderProgram, materialGold);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		mvStack.pop();

		// draw bunny/end of maze
		glUseProgram(toonShaderProgram);
		rt3d::setLightPos(toonShaderProgram, glm::value_ptr(tmp));
		rt3d::setUniformMatrix4fv(toonShaderProgram, "projection", glm::value_ptr(projection));
		mvStack.push(mvStack.top());
		mvStack.top() = glm::translate(mvStack.top(), glm::vec3(45.0f, 0.1f, 30.0f));
		mvStack.top() = glm::rotate(mvStack.top(), float(theta*DEG_TO_RADIAN), glm::vec3(0.0f, 0.5f, 0.0f));
		theta += 1.0f;
		mvStack.top() = glm::scale(mvStack.top(), glm::vec3(25.0, 25.0, 25.0));
		rt3d::setUniformMatrix4fv(toonShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
		rt3d::setMaterial(toonShaderProgram, material1);
		rt3d::drawIndexedMesh(meshObjects[2], toonIndexCount, GL_TRIANGLES);
		mvStack.pop();

		//Score: label at the top of the screen
		glUseProgram(textureProgram);
		rt3d::setUniformMatrix4fv(textureProgram, "projection", glm::value_ptr(projection));
		mvStack.pop();
		glDepthMask(GL_TRUE);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, labels[0]);
		                                                                                                           
		modelview = glm::translate(modelview, glm::vec3(-3.2f, 2.5f, -5.0f));                                 
		modelview = glm::scale(modelview, glm::vec3(0.5f, 0.5f, 0.0f));                                        
		rt3d::setUniformMatrix4fv(textureProgram, "modelview", glm::value_ptr(modelview));
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

	     //
		//PBOs drawn from here on to produce the blurring effect.
	   //

		glUseProgram(textureProgram);
		// could use an identity matrix for projection matrix... but not just now
		rt3d::setUniformMatrix4fv(textureProgram, "projection", glm::value_ptr(projection));
		// Copy the back buffer pixels directly to the PBO
		glBindTexture(GL_TEXTURE_2D, pboTex);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pboBuffer);
		glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		// Now to copy from PBO to the texture that has been set up
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		// Reset the model view matrix and draw a textured quad
		modelview = glm::translate(glm::mat4(1.0),
			glm::vec3(0.0f, -0.0f, -10.0f));
		// If using the cube mesh for rendering, the texture will be inverted
		// simple solution is to scale y by a negative value!
		modelview = glm::scale(modelview, glm::vec3(7.5f, -5.5f, 0.0f));
		rt3d::setUniformMatrix4fv(textureProgram, "modelview", glm::value_ptr(modelview));
		GLuint uniformIndex = glGetUniformLocation(textureProgram, "textureUnit0");
		glUniform1i(uniformIndex, 0);
		// Now bind textures to texture unit0 & draw the quad
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pboTex);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		rt3d::drawIndexedMesh(meshObjects[0], 6, GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);	// unbind the texture
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		
		modelview = glm::translate(glm::mat4(1.0),
			glm::vec3(0.0f, -0.0f, -10.0f));
		modelview = glm::scale(modelview, glm::vec3(7.3f, -5.3f, 0.0f));
		rt3d::setUniformMatrix4fv(textureProgram, "modelview", glm::value_ptr(modelview));
		uniformIndex = glGetUniformLocation(textureProgram, "textureUnit0");
		glUniform1i(uniformIndex, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pboTex);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		rt3d::drawIndexedMesh(meshObjects[0], 6, GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);	
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		
		modelview = glm::translate(glm::mat4(1.0),
			glm::vec3(0.0f, -0.0f, -10.0f));
		modelview = glm::scale(modelview, glm::vec3(7.1f, -5.1f, 0.0f));
		rt3d::setUniformMatrix4fv(textureProgram, "modelview", glm::value_ptr(modelview));
		uniformIndex = glGetUniformLocation(textureProgram, "textureUnit0");
		glUniform1i(uniformIndex, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pboTex);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		rt3d::drawIndexedMesh(meshObjects[0], 6, GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);	
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		
		modelview = glm::translate(glm::mat4(1.0),
			glm::vec3(0.0f, -0.0f, -10.0f));
		modelview = glm::scale(modelview, glm::vec3(6.9f, -4.9f, 0.0f));
		rt3d::setUniformMatrix4fv(textureProgram, "modelview", glm::value_ptr(modelview));
		uniformIndex = glGetUniformLocation(textureProgram, "textureUnit0");
		glUniform1i(uniformIndex, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pboTex);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		rt3d::drawIndexedMesh(meshObjects[0], 6, GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);	
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		
		modelview = glm::translate(glm::mat4(1.0),
			glm::vec3(0.0f, -0.0f, -10.0f));
		modelview = glm::scale(modelview, glm::vec3(6.7f, -4.7f, 0.0f));
		rt3d::setUniformMatrix4fv(textureProgram, "modelview", glm::value_ptr(modelview));
		uniformIndex = glGetUniformLocation(textureProgram, "textureUnit0");
		glUniform1i(uniformIndex, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pboTex);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		rt3d::drawIndexedMesh(meshObjects[0], 6, GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);	
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		
		modelview = glm::translate(glm::mat4(1.0),
			glm::vec3(0.0f, -0.0f, -10.0f));
		modelview = glm::scale(modelview, glm::vec3(6.5f, -4.5f, 0.0f));
		rt3d::setUniformMatrix4fv(textureProgram, "modelview", glm::value_ptr(modelview));
		uniformIndex = glGetUniformLocation(textureProgram, "textureUnit0");
		glUniform1i(uniformIndex, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pboTex);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		rt3d::drawIndexedMesh(meshObjects[0], 6, GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);	
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);


		SDL_GL_SwapWindow(window); // swap buffers
	}
