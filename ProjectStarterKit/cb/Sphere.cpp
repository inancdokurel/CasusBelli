
#include "Sphere.hpp"

#include "Triangle.hpp"
#include <iostream>
namespace cb {
	Sphere::Sphere() {

	}

	Sphere::Sphere(glm::vec4 color, glm::vec3 position, float radius) {
		this->_color = color;
		this->_position = position;
		this->_radius = radius;
	}

	glm::vec3 Sphere::normalize(glm::vec3 input) {
		glm::vec3 result = glm::normalize(input - _position);
		return result * _radius;
	}

	GLfloat * Sphere::render(int depth) {
		Triangle triangle1(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		Triangle triangle2(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		Triangle triangle3(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));
		Triangle triangle4(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));
		Triangle triangle5(glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		Triangle triangle6(glm::vec3(1, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		Triangle triangle7(glm::vec3(1, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
		Triangle triangle8(glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));

		GLfloat * triange1Data = triangle1.getTriangleData(depth);
		GLfloat * triange2Data = triangle2.getTriangleData(depth);
		GLfloat * triange3Data = triangle3.getTriangleData(depth);
		GLfloat * triange4Data = triangle4.getTriangleData(depth);
		GLfloat * triange5Data = triangle5.getTriangleData(depth);
		GLfloat * triange6Data = triangle6.getTriangleData(depth);
		GLfloat * triange7Data = triangle7.getTriangleData(depth);
		GLfloat * triange8Data = triangle8.getTriangleData(depth);

		int size = 3 * 3 * pow(4, depth);
		GLfloat * vertexData = new GLfloat[size * 8];

		std::copy(triange1Data, triange1Data + size, vertexData);
		std::copy(triange2Data, triange2Data + size, vertexData + size);
		std::copy(triange3Data, triange3Data + size, vertexData + size * 2);
		std::copy(triange4Data, triange4Data + size, vertexData + size * 3);
		std::copy(triange5Data, triange5Data + size, vertexData + size * 4);
		std::copy(triange6Data, triange6Data + size, vertexData + size * 5);
		std::copy(triange7Data, triange7Data + size, vertexData + size * 6);
		std::copy(triange8Data, triange8Data + size, vertexData + size * 7);

		free(triange1Data);
		free(triange2Data);
		free(triange3Data);
		free(triange4Data);
		free(triange5Data);
		free(triange6Data);
		free(triange7Data);
		free(triange8Data);

		GLfloat * vertexDataAugment = new GLfloat[size * 64 / 3];
		for (int i = 0, k = 0; i < size * 8;i = i + 3, k = k + 8) {
			//Normalize triangle according to center of sphere to achive concavity
			glm::vec3 result = normalize(glm::vec3(vertexData[i], vertexData[i + 1], vertexData[i + 2]));

			vertexDataAugment[k] = result.x;
			vertexDataAugment[k + 1] = result.y;
			vertexDataAugment[k + 2] = result.z;
		}

		for (int i = 0; i < size * 64 / 3; i = i + 24) {
			glm::vec3 x(vertexDataAugment[i], vertexDataAugment[i + 1], vertexDataAugment[i + 2]);
			glm::vec3 y(vertexDataAugment[i + 8], vertexDataAugment[i + 9], vertexDataAugment[i + 10]);
			glm::vec3 z(vertexDataAugment[i + 16], vertexDataAugment[i + 17], vertexDataAugment[i + 18]);
			glm::vec3 normal = glm::normalize(glm::cross(x - y, z - y));
			if (glm::dot(normal, x) < 0) {
				normal = -normal;
			}
			vertexDataAugment[i + 3] = 0.0f;
			vertexDataAugment[i + 4] = 0.0f;
			vertexDataAugment[i + 5] = normal.x;
			vertexDataAugment[i + 6] = normal.y;
			vertexDataAugment[i + 7] = normal.z;

			vertexDataAugment[i + 11] = 0.0f;
			vertexDataAugment[i + 12] = 1.0f;
			vertexDataAugment[i + 13] = normal.x;
			vertexDataAugment[i + 14] = normal.y;
			vertexDataAugment[i + 15] = normal.z;

			vertexDataAugment[i + 19] = 1.0f;
			vertexDataAugment[i + 20] = 0.0f;
			vertexDataAugment[i + 21] = normal.x;
			vertexDataAugment[i + 22] = normal.y;
			vertexDataAugment[i + 23] = normal.z;
		}

		return vertexDataAugment;
	}
}
