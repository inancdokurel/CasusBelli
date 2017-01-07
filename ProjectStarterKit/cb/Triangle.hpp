#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdio.h>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace cb {
	class Triangle {
	private:
		glm::vec3 _x, _y, _z;
		GLfloat * subdivide(float depth, glm::vec3 x, glm::vec3 y, glm::vec3 z);
	public:
		Triangle();
		Triangle(glm::vec3 x, glm::vec3 y, glm::vec3 z);
		GLfloat* getTriangleData(float depth);
	};

}
#endif

