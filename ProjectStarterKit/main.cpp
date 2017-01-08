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

// cb classes
#include "cb/Program.h"
#include "cb/Texture.h"
#include "cb/Camera.h"
#include "cb/Structures.h"
#include "cb/Tank.h"
#include "cb/Sphere.hpp"
#include "cb/Projectile.h"

# define PI          3.141592653589793238462643383279502884L

/*
Represents a textured geometry asset

Contains everything necessary to draw arbitrary geometry with a single texture:

- shaders
- a texture
- a VBO
- a VAO
- the parameters to glDrawArrays (drawType, drawStart, drawCount)
*/

using namespace cb;

// constants
const glm::vec2 SCREEN_SIZE(1366, 768);

const GLfloat TURN_RATE = 0.07;
const GLfloat MOVEMENT_RATE = 0.01;
const GLfloat MAX_ATTACK_DISTANCE = 40;
const GLfloat PROJECTILE_SPEED = 10;
const GLfloat GRAVITY = 1;
const GLfloat TURRET_HORIZONTAL_RATE = 0.1f;
const GLfloat TURRET_VERTICAL_RATE = 0.1f;

// globals
GLFWwindow* gWindow = NULL;
double gScrollY = 0.0;
cb::Camera gCamera;
cb::ModelAsset gTank;
cb::ModelAsset gTerrain;
cb::ModelAsset gBall;
std::vector<ModelInstance*> gInstances;
GLfloat gForward = 0.0f;
std::vector<cb::Light> gLights;
GLfloat mRight = 0.0f;
GLfloat mUp = 0.0f;
GLfloat camx;
GLfloat camy;
GLfloat camz;
ModelInstance terrain, tank1, tank2;
Tank pTank, eTank;
Projectile p;
std::vector<Projectile*> projectiles;

void AIMove(Tank& tank);
void ProjectileMove(float t);


// returns a new cb::Program created from the given vertex and fragment shader filenames
static cb::Program* LoadShaders(const char* vertFilename, const char* fragFilename) {
	std::vector<cb::Shader> shaders;
	shaders.push_back(cb::Shader::shaderFromFile(ResourcePath(vertFilename), GL_VERTEX_SHADER));
	shaders.push_back(cb::Shader::shaderFromFile(ResourcePath(fragFilename), GL_FRAGMENT_SHADER));
	return new cb::Program(shaders);
}


// returns a new cb::Texture created from the given filename
static cb::Texture* LoadTexture(const char* filename) {
	cb::Bitmap bmp = cb::Bitmap::bitmapFromFile(ResourcePath(filename));
	bmp.flipVertically();
	return new cb::Texture(bmp);
}


// initialises the gWoodenCrate global
static void LoadBoxAsset() {
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
void LoadBallAsset(int depth, ModelAsset & gBall) {

	// set all the elem ents of gWoodenCrate

	gBall.shaders = LoadShaders("vertex-shader.txt", "fragment-shader.txt");

	gBall.drawType = GL_TRIANGLES;

	gBall.drawStart = 0;

	gBall.drawCount = 8 * pow(4, depth) * 3;

	gBall.texture = LoadTexture("wooden-crate.jpg");

	gBall.shininess = 80.0;

	gBall.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);

	glGenBuffers(1, &gBall.vbo);

	glGenVertexArrays(1, &gBall.vao);



	// bind the VAO

	glBindVertexArray(gBall.vao);



	// bind the VBO

	glBindBuffer(GL_ARRAY_BUFFER, gBall.vbo);



	// Make a cube out of triangles (two triangles per side)

	Sphere sphere(glm::vec4(), glm::vec3(0, 0, 0), 1.0f);

	GLfloat* vertexDataPointer = sphere.render(depth);



	glBufferData(GL_ARRAY_BUFFER, 64 * 3 * pow(4, depth + 1), vertexDataPointer, GL_STATIC_DRAW);



	// connect the xyz to the "vert" attribute of the vertex shader

	glEnableVertexAttribArray(gBall.shaders->attrib("vert"));

	glVertexAttribPointer(gBall.shaders->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), NULL);



	// connect the uv coords to the "vertTexCoord" attribute of the vertex shader

	glEnableVertexAttribArray(gBall.shaders->attrib("vertTexCoord"));

	glVertexAttribPointer(gBall.shaders->attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));



	// connect the normal to the "vertNormal" attribute of the vertex shader

	glEnableVertexAttribArray(gBall.shaders->attrib("vertNormal"));

	glVertexAttribPointer(gBall.shaders->attrib("vertNormal"), 3, GL_FLOAT, GL_TRUE, 8 * sizeof(GLfloat), (const GLvoid*)(5 * sizeof(GLfloat)));



	// unbind the VAO

	glBindVertexArray(0);

}




//create all the `instance` structs for the 3D scene, and add them to `gInstances`
static void CreateInstances() {
	/*ModelInstance dot;
	dot.asset = &gTank;
	dot.transform = glm::mat4();
	gInstances.push_back(dot);*/


	terrain.asset = &gTerrain;
	terrain.transform = translate(-1024, 0, -1024)*scale(2048, 0, 2048);
	gInstances.push_back(&terrain);
	


	pTank = Tank(0, 0.5, 0, gTank, gTerrain, gTank, 0);

	eTank = Tank(0, 0.5, -120, gTank, gTerrain, gTank, 0);

	p = Projectile(0, 2, -1, gBall, 0, -10);
	gInstances.push_back(p.getBody());

	gInstances.push_back(pTank.GetBody());
	gInstances.push_back(pTank.GetTurret());
	gInstances.push_back(pTank.GetCannon());

	gInstances.push_back(eTank.GetBody());
	gInstances.push_back(eTank.GetTurret());
	gInstances.push_back(eTank.GetCannon());
	eTank.moveTurret(-10, 0);

	tank1.asset = &gTank;
	tank1.transform = translate(-5, 1, -10)*scale(1, 1, 1);
	tank1.positionX = -5;
	tank1.positionY = 1;
	tank1.positionZ = -10;
	gInstances.push_back(&tank1);


	tank2.asset = &gTank;
	tank2.transform = translate(5, 1, -10)*scale(1, 1, 1);
	tank2.positionX = 5;
	tank2.positionY = 1;
	tank2.positionZ = -10;
	gInstances.push_back(&tank2);


}

template <typename T>
void SetLightUniform(cb::Program* shaders, const char* propertyName, size_t lightIndex, const T& value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	std::string uniformName = ss.str();

	shaders->setUniform(uniformName.c_str(), value);
}

//renders a single `ModelInstance`
static void RenderInstance(ModelInstance* inst) {
	cb::ModelAsset* asset = inst->asset;
	cb::Program* shaders = asset->shaders;

	//bind the shaders
	shaders->use();

	//set the shader uniforms
	shaders->setUniform("camera", gCamera.matrix());
	shaders->setUniform("model", inst->transform);
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
	for (int i = 0; i < gInstances.size(); i++) {
		RenderInstance(gInstances[i]);
	}

	// swap the display buffers (displays what was just drawn)
	glfwSwapBuffers(gWindow);
}

// update the scene based on the time elapsed since last update
static void Update(float secondsElapsed) {
	//rotate the first instance in `gInstances`
	AIMove(eTank);
	const GLfloat degreesPerSecond = 180.0f;

	camx = pTank.GetBody()->positionX - 5 * glm::sin(glm::radians(mRight));
	camz = pTank.GetBody()->positionZ + 5 * glm::cos(glm::radians(mRight));
	camy = 2 + 2 * glm::sin(glm::radians(mUp));

	gCamera.setPosition(glm::vec3(camx, camy, camz));
	const float mouseSensitivity = 0.1f;
	double mouseX, mouseY;

	//move position of camera based on WASD keys, and XZ keys for up and down
	const float moveSpeed = 4.0; //units per second
	if (glfwGetKey(gWindow, 'S')) {
		pTank.moveBack(MOVEMENT_RATE);
	}
	else if (glfwGetKey(gWindow, 'W')) {
		pTank.move(MOVEMENT_RATE);
	}
	if (glfwGetKey(gWindow, 'A')) {
		pTank.rotateBody(TURN_RATE);
	}
	else if (glfwGetKey(gWindow, 'D')) {
		pTank.rotateBody(-TURN_RATE);
	}
	if (glfwGetKey(gWindow, 'Z')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * -glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(gWindow, 'X')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(gWindow, 'I')) {
		pTank.removeHealth(10);
	}
	else if (glfwGetKey(gWindow, 'J')) {
		pTank.removeHealth(-10);
	}
	else if (glfwGetKey(gWindow, 'K')) {
		Projectile* shot = new Projectile(eTank.GetCannon()->positionX, eTank.GetCannon()->positionY, eTank.GetCannon()->positionZ, gBall, eTank.getRightOrientation(), eTank.getUpOrientation());
		gInstances.push_back(shot->getBody());
		projectiles.push_back(shot);
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
	pTank.moveTurret(mUp, mRight);
	mRight = mRight + mouseSensitivity*(float)mouseX;
	if (!(mUp > 10 && mouseY > 0 || mUp < -45 && mouseY < 0))
	{
		mUp = mUp + mouseSensitivity*(float)mouseY;
		gCamera.offsetOrientation(mouseSensitivity * (float)mouseY, mouseSensitivity * (float)mouseX);
	}
	else {
		gCamera.offsetOrientation(0, mouseSensitivity * (float)mouseX);
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
	LoadBoxAsset();
	LoadBallAsset(6,gBall);

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
		ProjectileMove((float)(thisTime - lastTime));
		lastTime = thisTime;
		

		AIMove(eTank);
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
void AIMove(Tank& t) {

	GLfloat angleDifference = abs(pTank.getXZOrientation() - eTank.getXZOrientation());
	GLfloat distance = sqrt(pow(pTank.GetBody()->positionZ - t.GetBody()->positionZ, 2) + pow(pTank.GetBody()->positionX - t.GetBody()->positionX, 2));
	GLfloat minDistance = 30 + (pTank.getHealth() - t.getHealth())* 0.5;
	if (distance > minDistance) {
		if (abs(pTank.getXZOrientation() - eTank.getXZOrientation() - TURN_RATE) < angleDifference) {
			t.rotateBody(TURN_RATE);
		}
		else if (abs(pTank.getXZOrientation() - eTank.getXZOrientation() + TURN_RATE) < angleDifference) {
			t.rotateBody(-TURN_RATE);
		}
		else {
			t.rotateBody(TURN_RATE);
		}
		if (distance > sqrt(pow(pTank.GetBody()->positionZ - t.GetBody()->positionZ + MOVEMENT_RATE*glm::cos(glm::radians(t.getXZOrientation())), 2) + pow(pTank.GetBody()->positionX - t.GetBody()->positionX - MOVEMENT_RATE*glm::sin(glm::radians(-t.getXZOrientation())), 2)))
		{
			t.move(MOVEMENT_RATE);
		}
		else
		{
			t.moveBack(MOVEMENT_RATE);
		}
	}
	else{
		if (abs(pTank.getXZOrientation() - eTank.getXZOrientation() - TURN_RATE) < angleDifference) {
			t.rotateBody(-TURN_RATE);
		}
		else if (abs(pTank.getXZOrientation() - eTank.getXZOrientation() + TURN_RATE) < angleDifference) {
			t.rotateBody(+TURN_RATE);
		}
		else {
			t.rotateBody(TURN_RATE);
		}
		if (distance > sqrt(pow(pTank.GetBody()->positionZ - t.GetBody()->positionZ + MOVEMENT_RATE*glm::cos(glm::radians(t.getXZOrientation())), 2) + pow(pTank.GetBody()->positionX - t.GetBody()->positionX - MOVEMENT_RATE*glm::sin(glm::radians(-t.getXZOrientation())), 2)))
		{
			t.moveBack(MOVEMENT_RATE);
		}
		else
		{
			t.move(MOVEMENT_RATE);
		}
	}

	GLfloat aimDistance = sqrt(pow(pTank.GetBody()->positionZ - (t.GetTurret()->positionZ-glm::cos(glm::radians(t.getRightOrientation()))), 2) + pow(pTank.GetBody()->positionX - (t.GetTurret()->positionX + glm::sin(glm::radians(t.getRightOrientation()))), 2));
	if (aimDistance > sqrt(pow(pTank.GetBody()->positionZ - (t.GetTurret()->positionZ - glm::cos(glm::radians(t.getRightOrientation() + TURRET_HORIZONTAL_RATE))), 2) + pow(pTank.GetBody()->positionX - (t.GetTurret()->positionX + glm::sin(glm::radians(t.getRightOrientation() + TURRET_HORIZONTAL_RATE))), 2))){
		t.moveTurret(t.getUpOrientation(), t.getRightOrientation() + TURRET_HORIZONTAL_RATE);
	}
	else if(aimDistance < sqrt(pow(pTank.GetBody()->positionZ - (t.GetTurret()->positionZ - glm::cos(glm::radians(t.getRightOrientation() + TURRET_HORIZONTAL_RATE))), 2) + pow(pTank.GetBody()->positionX - (t.GetTurret()->positionX + glm::sin(glm::radians(t.getRightOrientation() + TURRET_HORIZONTAL_RATE))), 2))) {
		t.moveTurret(t.getUpOrientation(), t.getRightOrientation() - TURRET_HORIZONTAL_RATE);
	}

	
	float sin2theta = (distance*GRAVITY) / (PROJECTILE_SPEED * PROJECTILE_SPEED);
	float theta2 = asin(sin2theta);
	float angleToFire = ((theta2* 180.0f) / PI) / 2;
	if (angleToFire != angleToFire)
		angleToFire = t.getUpOrientation();
	if (angleToFire > -1 * (t.getUpOrientation())) {
		t.moveTurret(t.getUpOrientation() - TURRET_VERTICAL_RATE, t.getRightOrientation());
	}
	else if (angleToFire < -1 * (t.getUpOrientation())) {
		t.moveTurret(t.getUpOrientation() + TURRET_VERTICAL_RATE, t.getRightOrientation());
	}
	
	
	
	if (distance < MAX_ATTACK_DISTANCE) {
		if (t.shoot()) {
			Projectile* shot= new Projectile(t.GetCannon()->positionX, t.GetCannon()->positionY, t.GetCannon()->positionZ, gBall, t.getRightOrientation(), t.getUpOrientation());
			gInstances.push_back(shot->getBody());
			projectiles.push_back(shot);
		}
	}
	
}
void ProjectileMove(float secondsEllapsed) {
	for (int i = 0;i < projectiles.size();i++) {
		projectiles[i]->move(secondsEllapsed, GRAVITY, PROJECTILE_SPEED);
	}
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
