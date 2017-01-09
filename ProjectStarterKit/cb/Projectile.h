#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "Structures.h"
namespace cb {
	class Projectile {
	public:
		Projectile() {
			x = 0;
			y = 0;
			z = 0;
			xzAngle = 0;
			body = nullptr;
			yAngle = 0;
			timeEllapsed = 0;
			currentX = x;
			currentY = y;
			currentZ = z;
		}
		Projectile(GLfloat xPos, GLfloat yPos, GLfloat zPos, ModelAsset & bodyAsset, GLfloat xzA, GLfloat yA) {
			x = xPos;
			y = yPos;
			z = zPos;
			currentX = x;
			currentY = y;
			currentZ = z;
			xzAngle = xzA;
			yAngle = yA;
			body = new ModelInstance();
			body->asset = &bodyAsset;
			body->transform = translate(x, y, z)*scale(0.1f, 0.1f, 0.1f);
			body->positionX = x;
			body->positionY = y;
			body->positionZ = z;
			timeEllapsed = 0;
		}
		ModelInstance*  getBody(){ return body;}
		void move(float t,float g,float v) {
			timeEllapsed += t;
			currentX = x + glm::cos(glm::radians(-yAngle))*glm::sin(glm::radians(xzAngle))*v*timeEllapsed;
			currentY = y + v*timeEllapsed*glm::sin(glm::radians(-yAngle)) - 0.5f*timeEllapsed*timeEllapsed*g;
			currentZ = z - glm::cos(glm::radians(-yAngle))*glm::cos(glm::radians(xzAngle))*v*timeEllapsed;
			body->transform=translate(currentX,currentY,currentZ)*scale(0.1f,0.1f,0.1f);
		
		}
		GLfloat getX() { return currentX; }
		GLfloat getY() { return currentY; }
		GLfloat getZ() { return currentZ; }
	private:
		ModelInstance* body;
		GLfloat x, y, z,currentX,currentY,currentZ;
		GLfloat xzAngle = 0;
		GLfloat yAngle = 0;
		GLfloat timeEllapsed = 0;
	};


}
#endif