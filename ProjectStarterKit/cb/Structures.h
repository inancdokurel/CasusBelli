#pragma once
#include "platform.hpp"
#include<glm/gtc/type_ptr.hpp>


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <list>
#include <sstream>

#include "Program.h"
#include "Texture.h"
#include "Camera.h"

namespace cb {

	struct ModelAsset {
		cb::Program* shaders;
		cb::Texture* texture;
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
}