
#include "Triangle.hpp"
#include <cmath>
#include <iostream>
namespace cb {
	Triangle::Triangle() :
		_x(glm::vec3()), _y(glm::vec3()), _z(glm::vec3())
	{
	}

	Triangle::Triangle(glm::vec3 x, glm::vec3 y, glm::vec3 z) :
		_x(x), _y(y), _z(z)
	{
	}

	GLfloat * Triangle::subdivide(float depth, glm::vec3 x, glm::vec3 y, glm::vec3 z) {
		//End of the recursion
		if (depth == 0) {
			GLfloat * result = new GLfloat[3 * 3];

			result[0] = x.x;
			result[1] = x.y;
			result[2] = x.z;

			result[3] = y.x;
			result[4] = y.y;
			result[5] = y.z;

			result[6] = z.x;
			result[7] = z.y;
			result[8] = z.z;

			return result;
		}

		GLfloat * upper = subdivide(depth - 1, x, (x + y) / 2.0f, (x + z) / 2.0f);
		GLfloat * left = subdivide(depth - 1, y, (x + y) / 2.0f, (y + z) / 2.0f);
		GLfloat * right = subdivide(depth - 1, z, (x + z) / 2.0f, (y + z) / 2.0f);
		GLfloat * middle = subdivide(depth - 1, (x + y) / 2.0f, (x + z) / 2.0f, (y + z) / 2.0f);

		GLfloat * result = new GLfloat[(int)pow(4, depth) * 9];

		int size = (int)pow(4, depth - 1) * 9;
		std::copy(upper, upper + size, result);
		std::copy(left, left + size, result + size);
		std::copy(right, right + size, result + size + size);
		std::copy(middle, middle + size, result + size + size + size);

		return result;
	}

	GLfloat * Triangle::getTriangleData(float depth) {
		return subdivide(depth, _x, _y, _z);
	}

}
