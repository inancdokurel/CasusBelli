/*
main

Copyright 2012 Thomas Dalling - http://tomdalling.com/

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#define GLEW_STATIC
#include "platform.hpp"
#include<glm/gtc/type_ptr.hpp>

// third-party libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// standard C++ libraries
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <list>
#include <sstream>

// tdogl classes
#include "tdogl/Program.h"
#include "tdogl/Texture.h"
#include "tdogl/Camera.h"

/*
Represents a textured geometry asset

Contains everything necessary to draw arbitrary geometry with a single texture:

- shaders
- a texture
- a VBO
- a VAO
- the parameters to glDrawArrays (drawType, drawStart, drawCount)
*/
struct ModelAsset {
	tdogl::Program* shaders;
	tdogl::Texture* texture;
	GLuint vbo;
	GLuint vao;
	GLenum drawType;
	GLint drawStart;
	GLint drawCount;
	GLfloat shininess;
	glm::vec3 specularColor;

	ModelAsset() :
		shaders(NULL),
		texture(NULL),
		vbo(0),
		vao(0),
		drawType(GL_TRIANGLES),
		drawStart(0),
		drawCount(0),
		shininess(0.0f),
		specularColor(1.0f, 1.0f, 1.0f)
	{}
};

/*
Represents an instance of an `ModelAsset`

Contains a pointer to the asset, and a model transformation matrix to be used when drawing.
*/

struct ModelInstance {
	ModelAsset* asset;
	glm::mat4 transform;
	GLfloat positionX;
	GLfloat positionY;
	GLfloat positionZ;
	void setPosition(GLfloat x, GLfloat y, GLfloat z);
	ModelInstance() :
		asset(NULL),
		transform()
	{}
};


void ModelInstance::setPosition(GLfloat x, GLfloat y, GLfloat z)
{
	positionX = x;
	positionY = y;
	positionZ = z;
}

/*
Represents a point light
*/
struct Light {
	glm::vec4 position;
	glm::vec3 intensities; //a.k.a. the color of the light
	float attenuation;
	float ambientCoefficient;
	float coneAngle;
	glm::vec3 coneDirection;
};

// constants
const glm::vec2 SCREEN_SIZE(1366, 768);

// globals
GLFWwindow* gWindow = NULL;
double gScrollY = 0.0;
tdogl::Camera gCamera;
ModelAsset gTank;
ModelAsset gTerrain;
ModelInstance * gInstances = new ModelInstance[10];
GLfloat gDegreesRotated = 0.0f;
GLfloat gRight = 0.0f;
GLfloat gForward = 0.0f;
std::vector<Light> gLights;
GLfloat mRight = 0.0f;
GLfloat mUp = 0.0f;
GLfloat camx;
GLfloat camy;
GLfloat camz;



// returns a new tdogl::Program created from the given vertex and fragment shader filenames
static tdogl::Program* LoadShaders(const char* vertFilename, const char* fragFilename) {
	std::vector<tdogl::Shader> shaders;
	shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(vertFilename), GL_VERTEX_SHADER));
	shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath(fragFilename), GL_FRAGMENT_SHADER));
	return new tdogl::Program(shaders);
}


// returns a new tdogl::Texture created from the given filename
static tdogl::Texture* LoadTexture(const char* filename) {
	tdogl::Bitmap bmp = tdogl::Bitmap::bitmapFromFile(ResourcePath(filename));
	bmp.flipVertically();
	return new tdogl::Texture(bmp);
}


// initialises the gWoodenCrate global
static void LoadTankSteelAsset() {
	// set all the elements of gWoodenCrate

	gTerrain.shaders = LoadShaders("vertex-shader.txt", "fragment-shader.txt");
	gTerrain.drawType = GL_TRIANGLES;
	gTerrain.drawStart = 0;
	gTerrain.drawCount = 6 * 2 * 3;
	gTerrain.texture = LoadTexture("terrain.jpg");
	gTerrain.shininess = 80.0;
	gTerrain.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glGenBuffers(1, &gTerrain.vbo);
	glGenVertexArrays(1, &gTerrain.vao);

	// bind the VAO
	glBindVertexArray(gTerrain.vao);

	// bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, gTerrain.vbo);

	// Make a cube out of triangles (two triangles per side)
	GLfloat vertexData[] = {
		//  X     Y     Z       U     V          Normal
		// bottom
		-1.0f,-1.0f,-1.0f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
		1.0f,-1.0f, 1.0f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f,

		// top
		-1.0f, 1.0f,-1.0f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,

		// front
		-1.0f,-1.0f, 1.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		1.0f,-1.0f, 1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
		1.0f,-1.0f, 1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

		// back
		-1.0f,-1.0f,-1.0f,   0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
		-1.0f, 1.0f,-1.0f,   0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
		-1.0f, 1.0f,-1.0f,   0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
		1.0f, 1.0f,-1.0f,   1.0f, 1.0f,   0.0f, 0.0f, -1.0f,

		// left
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
		-1.0f,-1.0f,-1.0f,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f,-1.0f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

		// right
		1.0f,-1.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		1.0f, 1.0f,-1.0f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		1.0f,-1.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
		1.0f, 1.0f,-1.0f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	// connect the xyz to the "vert" attribute of the vertex shader
	glEnableVertexAttribArray(gTerrain.shaders->attrib("vert"));
	glVertexAttribPointer(gTerrain.shaders->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), NULL);

	// connect the uv coords to the "vertTexCoord" attribute of the vertex shader
	glEnableVertexAttribArray(gTerrain.shaders->attrib("vertTexCoord"));
	glVertexAttribPointer(gTerrain.shaders->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

	// connect the normal to the "vertNormal" attribute of the vertex shader
	glEnableVertexAttribArray(gTerrain.shaders->attrib("vertNormal"));
	glVertexAttribPointer(gTerrain.shaders->attrib("vertNormal"), 3, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (const GLvoid*)(5 * sizeof(GLfloat)));

	// unbind the VAO
	glBindVertexArray(0);

	gTank.shaders = LoadShaders("vertex-shader.txt", "fragment-shader.txt");
	gTank.drawType = GL_TRIANGLES;
	gTank.drawStart = 0;
	gTank.drawCount = 6 * 2 * 3;
	gTank.texture = LoadTexture("wooden-crate.jpg");
	gTank.shininess = 80.0;
	gTank.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glGenBuffers(1, &gTank.vbo);
	glGenVertexArrays(1, &gTank.vao);

	// bind the VAO
	glBindVertexArray(gTank.vao);

	// bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, gTank.vbo);

	// Make a cube out of triangles (two triangles per side)

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	// connect the xyz to the "vert" attribute of the vertex shader
	glEnableVertexAttribArray(gTank.shaders->attrib("vert"));
	glVertexAttribPointer(gTank.shaders->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), NULL);

	// connect the uv coords to the "vertTexCoord" attribute of the vertex shader
	glEnableVertexAttribArray(gTank.shaders->attrib("vertTexCoord"));
	glVertexAttribPointer(gTank.shaders->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

	// connect the normal to the "vertNormal" attribute of the vertex shader
	glEnableVertexAttribArray(gTank.shaders->attrib("vertNormal"));
	glVertexAttribPointer(gTank.shaders->attrib("vertNormal"), 3, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (const GLvoid*)(5 * sizeof(GLfloat)));

	// unbind the VAO
	glBindVertexArray(0);
}


void moveObject(ModelInstance m, GLfloat x, GLfloat y, GLfloat z) {

}

// convenience function that returns a translation matrix
glm::mat4 translate(GLfloat x, GLfloat y, GLfloat z) {
	return glm::translate(glm::mat4(), glm::vec3(x, y, z));
}


// convenience function that returns a scaling matrix
glm::mat4 scale(GLfloat x, GLfloat y, GLfloat z) {
	return glm::scale(glm::mat4(), glm::vec3(x, y, z));
}

glm::mat4 rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
	return glm::rotate(glm::mat4(), angle, glm::vec3(x, y, z));
}

//create all the `instance` structs for the 3D scene, and add them to `gInstances`
static void CreateInstances() {
	/*ModelInstance dot;
	dot.asset = &gTank;
	dot.transform = glm::mat4();
	gInstances.push_back(dot);*/

	ModelInstance terrain;
	terrain.asset = &gTerrain;
	terrain.transform = translate(-1024, 0, -1024)*scale(2048, 0, 2048);
	gInstances[0] = terrain;

	ModelInstance ptank;
	ptank.asset = &gTank;
	ptank.transform = translate(0, 0.5, 0)*scale(1.5, 0.5, 2);
	ptank.positionX = 0;
	ptank.positionY = 0.5;
	ptank.positionZ = 0;
	gInstances[1] = ptank;

	ModelInstance tankTurret;
	tankTurret.asset = &gTerrain;
	tankTurret.transform = translate(0, 0.75, 0)*scale(0.5, 0.75, 1);
	tankTurret.positionX = 0;
	tankTurret.positionY = 0.75;
	tankTurret.positionZ = 0;
	gInstances[2] = tankTurret;

	ModelInstance tank1;
	tank1.asset = &gTank;
	tank1.transform = translate(-5, 1, -10)*scale(1, 1, 1);
	tank1.positionX = -5;
	tank1.positionY = 1;
	tank1.positionZ = -10;
	gInstances[3] = tank1;


	ModelInstance tank2;
	tank2.asset = &gTank;
	tank2.transform = translate(5, 1, -10)*scale(1, 1, 1);
	tank2.positionX = 5;
	tank2.positionY = 1;
	tank2.positionZ = -10;
	gInstances[4] = tank2;

	ModelInstance tankCannon;
	tankCannon.asset = &gTank;
	tankCannon.transform = translate(0, 1.125, -1)*scale(0.1, 0.1, 2.25);
	tankCannon.positionX = 0;
	tankCannon.positionY = 1.125;
	tankCannon.positionZ = -1;
	gInstances[5] = tankCannon;
}

template <typename T>
void SetLightUniform(tdogl::Program* shaders, const char* propertyName, size_t lightIndex, const T& value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shaders->setUniform(uniformName.c_str(), value);
}

//renders a single `ModelInstance`
static void RenderInstance(const ModelInstance& inst) {
	ModelAsset* asset = inst.asset;
	tdogl::Program* shaders = asset->shaders;

	//bind the shaders
	shaders->use();

	//set the shader uniforms
	shaders->setUniform("camera", gCamera.matrix());
	shaders->setUniform("model", inst.transform);
	shaders->setUniform("materialTex", 0); //set to 0 because the texture will be bound to GL_TEXTURE0
	shaders->setUniform("materialShininess", asset->shininess);
	shaders->setUniform("materialSpecularColor", asset->specularColor);
	shaders->setUniform("cameraPosition", gCamera.position());
	shaders->setUniform("numLights", (int)gLights.size());

	for (size_t i = 0; i < gLights.size(); ++i) {
		SetLightUniform(shaders, "position", i, gLights[i].position);
		SetLightUniform(shaders, "intensities", i, gLights[i].intensities);
		SetLightUniform(shaders, "attenuation", i, gLights[i].attenuation);
		SetLightUniform(shaders, "ambientCoefficient", i, gLights[i].ambientCoefficient);
		SetLightUniform(shaders, "coneAngle", i, gLights[i].coneAngle);
		SetLightUniform(shaders, "coneDirection", i, gLights[i].coneDirection);
	}

	//bind the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, asset->texture->object());

	//bind VAO and draw
	glBindVertexArray(asset->vao);
	glDrawArrays(asset->drawType, asset->drawStart, asset->drawCount);

	//unbind everything
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	shaders->stopUsing();
}


// draws a single frame
static void Render() {
	// clear everything
	glClearColor(0.4, 0.4, 0.6, 1); // black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render all the instances
	std::list<ModelInstance>::const_iterator it;
	for (int i = 0; i < 6; i++) {
		RenderInstance(gInstances[i]);
	}

	// swap the display buffers (displays what was just drawn)
	glfwSwapBuffers(gWindow);
}

double* getArray(ModelInstance* gInstances, int j) {
	double dArray[16] = { 0.0 };
	const float *pSource = (const float*)glm::value_ptr(gInstances[j].transform);
	for (int i = 0; i < 16; ++i)
		dArray[i] = pSource[i];
	return dArray;
}

// update the scene based on the time elapsed since last update
static void Update(float secondsElapsed) {
	//rotate the first instance in `gInstances`
	const GLfloat degreesPerSecond = 180.0f;
	gDegreesRotated = secondsElapsed * gRight;
	while (gRight > 360.0f) gRight -= 360.0f;

	double * dArray = new double[16];
	dArray = getArray(gInstances, 1);
	double * tArray = new double[16];
	tArray = getArray(gInstances, 2);
	
	camx = gInstances[1].positionX - 5 * glm::sin(glm::radians(mRight));
	camz = gInstances[1].positionZ + 5 * glm::cos(glm::radians(mRight));
	camy = 2+2* glm::sin(glm::radians(mUp));

	gCamera.setPosition(glm::vec3(camx, camy, camz));
	glm::mat4 instance2translate;
	glm::mat4 instance2rotate= rotate(glm::radians(gRight), 0, 1, 0);
	glm::mat4 instance5translate;
	glm::mat4 instance5rotate= rotate(glm::radians(gRight), 0, 1, 0);
	const float mouseSensitivity = 0.1f;
	double mouseX, mouseY;
	//move position of camera based on WASD keys, and XZ keys for up and down
	const float moveSpeed = 4.0; //units per second
	if (glfwGetKey(gWindow, 'S')) {
		gForward += 0.01;
		gInstances[1].transform = translate(gInstances[1].positionX + 0.01*glm::sin(glm::radians(gRight)), gInstances[1].positionY, gInstances[1].positionZ + 0.01*glm::cos(glm::radians(gRight)))*rotate(glm::radians(gRight), 0, 1, 0)*scale(1.5, 0.5, 2);
		instance2translate = translate(gInstances[2].positionX + 0.01*glm::sin(glm::radians(gRight)), gInstances[2].positionY, gInstances[2].positionZ + 0.01*glm::cos(glm::radians(gRight)));
		gInstances[2].transform = instance2translate*instance2rotate*scale(0.5, 0.75, 1);
		instance5translate= translate(gInstances[5].positionX + 0.01*glm::sin(glm::radians(gRight)), gInstances[5].positionY, gInstances[5].positionZ + 0.01*glm::cos(glm::radians(gRight)));
		gInstances[5].transform = instance5translate*instance5rotate*scale(0.1, 0.1, 1);

		double *s1Array = new double[16];
		s1Array = getArray(gInstances, 1);
		gInstances[1].setPosition(s1Array[12], s1Array[13], s1Array[14]);

		double *s2Array = new double[16];
		s2Array = getArray(gInstances, 2);
		gInstances[2].setPosition(s2Array[12], s2Array[13], s2Array[14]);

		double *s5Array = new double[16];
		s5Array = getArray(gInstances, 5);
		gInstances[5].setPosition(s5Array[12], s5Array[13], s5Array[14]);
		


		//gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.forward());
	}
	else if (glfwGetKey(gWindow, 'W')) {
		gForward -= 0.01;
		gInstances[1].transform = translate(gInstances[1].positionX + 0.01*glm::sin(glm::radians(-gRight)), gInstances[1].positionY, gInstances[1].positionZ - 0.01*glm::cos(glm::radians(gRight)))*rotate(glm::radians(gRight), 0, 1, 0)*scale(1.5, 0.5, 2);

		instance2translate = translate(gInstances[2].positionX + 0.01*glm::sin(glm::radians(-gRight)), gInstances[2].positionY, gInstances[2].positionZ - 0.01*glm::cos(glm::radians(gRight)));
		
		gInstances[2].transform = instance2translate*instance2rotate*scale(0.5, 0.75, 1);
		
		instance5translate = translate(gInstances[5].positionX + 0.01*(glm::sin(glm::radians(gRight))), gInstances[5].positionY, gInstances[5].positionZ - 0.01*(1 - glm::cos(glm::radians(gRight))));
		gInstances[5].transform = instance5translate*instance5rotate*scale(0.1, 0.1, 1);

		//gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.forward());
		double *s1Array = new double[16];
		s1Array = getArray(gInstances, 1);
		gInstances[1].setPosition(s1Array[12], s1Array[13], s1Array[14]);

		double *s2Array = new double[16];
		s2Array = getArray(gInstances, 2);
		gInstances[2].setPosition(s2Array[12], s2Array[13], s2Array[14]);

		double *s5Array = new double[16];
		s5Array = getArray(gInstances, 5);
		gInstances[5].setPosition(s5Array[12], s5Array[13], s5Array[14]);
		std::cout << s5Array[12] << " " << s5Array[13] << " " << s5Array[14] << "   " << std::endl;
	}
	if (glfwGetKey(gWindow, 'A')) {
		gRight += 0.07;
		gInstances[1].transform = translate(gInstances[1].positionX, gInstances[1].positionY, gInstances[1].positionZ)*rotate(glm::radians(gRight), 0, 1, 0)*scale(1.5, 0.5, 2);
		

		//gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.right());
	}
	else if (glfwGetKey(gWindow, 'D')) {
		gRight -= 0.07;
		gInstances[1].transform = translate(gInstances[1].positionX, gInstances[1].positionY, gInstances[1].positionZ)*rotate(glm::radians(gRight), 0, 1, 0)*scale(1.5, 0.5, 2);
		std::cout << gInstances[5].positionX << std::endl;

		//gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.right());
	}
	if (glfwGetKey(gWindow, 'Z')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * -glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(gWindow, 'X')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * glm::vec3(0, 1, 0));
	}

	//move light
	if (glfwGetKey(gWindow, '1')) {
		gLights[0].position = glm::vec4(1000, 1000, 0, 1.0);
		gLights[0].coneDirection = glm::vec3(0, -1, 0);
	}

	// change light color
	if (glfwGetKey(gWindow, '2'))
		gLights[0].intensities = glm::vec3(2, 0, 0); //red
	else if (glfwGetKey(gWindow, '3'))
		gLights[0].intensities = glm::vec3(0, 2, 0); //green
	else if (glfwGetKey(gWindow, '4'))
		gLights[0].intensities = glm::vec3(2, 2, 2); //white


	 //rotate camera based on mouse movement



	glfwGetCursorPos(gWindow, &mouseX, &mouseY);
	gInstances[2].transform = translate(gInstances[2].positionX, gInstances[2].positionY, gInstances[2].positionZ)*rotate(glm::radians(-mRight), 0, 1, 0)*scale(0.5, 0.75, 1);
	gInstances[5].transform = translate(gInstances[2].positionX+glm::sin(glm::radians(mRight)) + 0.01*(glm::sin(glm::radians(gRight))), gInstances[2].positionY+0.325-glm::sin(glm::radians(mUp)), gInstances[2].positionZ- (glm::cos(glm::radians(-mRight))))*rotate(glm::radians(-mRight), 0, 1, 0)*rotate(-glm::radians(mUp),1,0,0)*scale(0.1, 0.1, 1.5);

	mRight = mRight + mouseSensitivity*(float)mouseX/10;
	if (!(mUp > 10 && mouseY > 0 || mUp < -45 && mouseY < 0))
	{
		mUp = mUp + mouseSensitivity*(float)mouseY/10;
		gCamera.offsetOrientation(mouseSensitivity * (float)mouseY/10, mouseSensitivity * (float)mouseX/10);
	}
	else {
		gCamera.offsetOrientation(0, mouseSensitivity * (float)mouseX/10);
	}
	
	glfwSetCursorPos(gWindow, 0, 0); //reset the mouse, so it doesn't go out of the window

									 //increase or decrease field of view based on mouse wheel
	const float zoomSensitivity = -0.2f;
	float fieldOfView = gCamera.fieldOfView() + zoomSensitivity * (float)gScrollY;
	if (fieldOfView < 5.0f) fieldOfView = 5.0f;
	if (fieldOfView > 130.0f) fieldOfView = 130.0f;
	gCamera.setFieldOfView(fieldOfView);
	gScrollY = 0;
}

// records how far the y axis has been scrolled
void OnScroll(GLFWwindow* window, double deltaX, double deltaY) {
	gScrollY += deltaY;
}

void OnError(int errorCode, const char* msg) {
	throw std::runtime_error(msg);
}

// the program starts here
void AppMain() {
	// initialise GLFW
	glfwSetErrorCallback(OnError);
	if (!glfwInit())
		throw std::runtime_error("glfwInit failed");

	// open a window with GLFW
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	gWindow = glfwCreateWindow((int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y, "Project Starter Kit", NULL, NULL);
	if (!gWindow)
		throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 3.2?");

	// GLFW settings
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(gWindow, 0, 0);
	glfwSetScrollCallback(gWindow, OnScroll);
	glfwMakeContextCurrent(gWindow);

	// initialise GLEW
	glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
	if (glewInit() != GLEW_OK)
		throw std::runtime_error("glewInit failed");

	// GLEW throws some errors, so discard all the errors so far
	while (glGetError() != GL_NO_ERROR) {}

	// print out some info about the graphics drivers
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	
	// make sure OpenGL version 3.2 API is available
	if (!GLEW_VERSION_3_2)
		throw std::runtime_error("OpenGL 3.2 API is not available.");

	// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// initialise the gWoodenCrate asset
	LoadTankSteelAsset();

	// create all the instances in the 3D scene based on the gWoodenCrate asset
	CreateInstances();

	// setup gCamera
	gCamera.setPosition(glm::vec3(0, 5, -14));
	gCamera.setViewportAspectRatio(SCREEN_SIZE.x / SCREEN_SIZE.y);
	gCamera.setNearAndFarPlanes(0.5f, 100.0f);

	// setup lights
	Light spotlight;
	spotlight.position = glm::vec4(-4, 0, 10, 1);
	spotlight.intensities = glm::vec3(2, 2, 2); //strong white light
	spotlight.attenuation = 0.1f;
	spotlight.ambientCoefficient = 0.5f; //no ambient light
	spotlight.coneAngle = 15.0f;
	spotlight.coneDirection = glm::vec3(0, 0, -1);

	Light directionalLight;
	directionalLight.position = glm::vec4(1, 0.8, 0.6, 0); //w == 0 indications a directional light
	directionalLight.intensities = glm::vec3(0.4, 0.3, 0.1); //weak yellowish light
	directionalLight.ambientCoefficient = 0.06f;

	gLights.push_back(spotlight);
	gLights.push_back(directionalLight);


	// run while the window is open
	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(gWindow)) {
		// process pending events
		glfwPollEvents();

		// update the scene based on the time elapsed since last update
		double thisTime = glfwGetTime();
		Update((float)(thisTime - lastTime));
		lastTime = thisTime;

		// draw one frame
		Render();

		// check for errors
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			std::cerr << "OpenGL Error " << error << std::endl;

		//exit program if escape key is pressed
		if (glfwGetKey(gWindow, GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(gWindow, GL_TRUE);
	}

	// clean up and exit
	glfwTerminate();
}


int main(int argc, char *argv[]) {
	try {
		AppMain();
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
