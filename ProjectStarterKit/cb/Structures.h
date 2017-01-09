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
		glm::vec3 Ax, Ay, Az;
		glm::vec3 size;
		glm::vec3 pos;

		
		void setPosition(GLfloat x, GLfloat y, GLfloat z);
		ModelInstance() :
			asset(NULL),
			transform()
		{}
		void setColllisionVectors(double* transformArray)
		{
			glm::vec3 RT(transformArray[0], transformArray[1], transformArray[2]);
			glm::vec3 UP(transformArray[4], transformArray[5], transformArray[6]);
			glm::vec3 BK(transformArray[8], transformArray[9], transformArray[10]);
			glm::vec3 POS(transformArray[12], transformArray[13], transformArray[14]);
			GLfloat RTnormal = sqrt(RT.x*RT.x + RT.y * RT.y + RT.z * RT.z);
			GLfloat UPnormal = sqrt(UP.x*UP.x + UP.y * UP.y + UP.z * UP.z);
			GLfloat BKnormal = sqrt(BK.x*BK.x + BK.y * BK.y + BK.z * BK.z);
			glm::vec3 RT2(transformArray[0] / RTnormal, transformArray[1] / RTnormal, transformArray[2] / RTnormal);
			glm::vec3 UP2(transformArray[4] / UPnormal, transformArray[5] / UPnormal, transformArray[6] / UPnormal);
			glm::vec3 BK2(transformArray[8] / BKnormal, transformArray[9] / BKnormal, transformArray[10] / BKnormal);
			Ax = RT2;
			Ay = UP2;
			Az = BK2;
			pos.x = POS.x;
			pos.y = POS.y;
			pos.z = POS.z;
		}
		void setSize(GLfloat x, GLfloat y, GLfloat z) {
			size.x = x;
			size.y = y;
			size.z = z;
		}
	};

	void ModelInstance::setPosition(GLfloat x, GLfloat y, GLfloat z)
	{
		positionX = x;
		positionY = y;
		positionZ = z;
	}
	double* getTransformArray(ModelInstance m) {
		double* dArray = new double[16];
		const float *pSource = (const float*)glm::value_ptr(m.transform);
		for (int i = 0; i < 16; ++i) {
			dArray[i] = pSource[i];
		}
		return dArray;
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

	glm::vec3 multiply(GLfloat a, glm::vec3 vec) {
		glm::vec3 newVec(a*vec.x, a*vec.y, a*vec.z);
		return newVec;
	}

}