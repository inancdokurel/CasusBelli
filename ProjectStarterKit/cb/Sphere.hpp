
#ifndef SPHERE_H
#define SPHERE_H

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace cb {
	class Sphere {
	private:
		glm::vec3 normalize(glm::vec3);
	public:
		glm::vec4 _color;
		glm::vec3 _position;
		float _radius;

		Sphere();
		Sphere(glm::vec4 color, glm::vec3 position, float radius);
		GLfloat * render(int);

	};
}

#endif
