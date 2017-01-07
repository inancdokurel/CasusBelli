#ifndef TANK_H
#define TANK_H
#include "Structures.h"
#include "Projectile.h"
#include <chrono>
const double MIN_ATTACK_INTERVAL = 2000;

namespace cb {

	class Tank {
	public:
		Tank() {
			x = 0;
			y = 0;
			z = 0;
			xzOrientation = 0;
			body = nullptr;
			turret = nullptr;
			cannon = nullptr;

			health = 100;
			lastAttack = std::chrono::steady_clock::now();
		}
		Tank(GLfloat xPos, GLfloat yPos, GLfloat zPos, ModelAsset & bodyAsset, ModelAsset & turretAsset, ModelAsset & cannonAsset,GLfloat xz) {
			x = xPos;
			y = yPos;
			z = zPos;
			xzOrientation = xz;
			body = new ModelInstance();
			body->asset = &bodyAsset;
			body->transform = translate(x, y, z)*scale(1.5, 0.5, 2);
			body->positionX = x;
			body->positionY = y;
			body->positionZ = z;

			turret = new ModelInstance();
			turret->asset = &turretAsset;
			turret->transform = translate(x, y + 0.25, z)*scale(0.5, 0.75, 1);
			turret->positionX = x;
			turret->positionY = y + 0.25;
			turret->positionZ = z;

			cannon = new ModelInstance();
			cannon->asset = &cannonAsset;
			cannon->transform = translate(x, y+0.675, z - 1)*scale(0.1, 0.1, 1);
			cannon->positionX = x;
			cannon->positionY = y+0.675;
			cannon->positionZ = z - 1;

			health = 100;
			lastAttack = std::chrono::steady_clock::now();
		}
		void rotateBody(GLfloat turnRate) {
			xzOrientation += turnRate;
			body->transform = translate(body->positionX, body->positionY, body->positionZ)*rotate(glm::radians(xzOrientation), 0, 1, 0)*scale(1.5, 0.5, 2);
		}
		ModelInstance* GetBody() { return body; }
		ModelInstance* GetTurret() { return turret; }
		ModelInstance* GetCannon() { return cannon; }
		void move(GLfloat movementRate) {
			body->transform = translate(body->positionX + movementRate*glm::sin(glm::radians(-xzOrientation) ), body->positionY, body->positionZ - movementRate*glm::cos(glm::radians(xzOrientation)))*rotate(glm::radians(xzOrientation), 0, 1, 0)*scale(1.5, 0.5, 2);
	
			body->setPosition(body->positionX + movementRate*glm::sin(glm::radians(-xzOrientation) ), body->positionY, body->positionZ - movementRate*glm::cos(glm::radians(xzOrientation)));
			turret->setPosition(turret->positionX + movementRate*glm::sin(glm::radians(-xzOrientation) ), turret->positionY, turret->positionZ - movementRate*glm::cos(glm::radians(xzOrientation)));
			cannon->setPosition(cannon->positionX + movementRate*(glm::sin(glm::radians(xzOrientation)) ), cannon->positionY+0.675, cannon->positionZ - movementRate*(1 - glm::cos(glm::radians(xzOrientation))));
			moveTurret(upOrientation, rightOrientation);
		}
		void moveBack(GLfloat movementRate) {
			body->transform = translate(body->positionX + movementRate*glm::sin(glm::radians(xzOrientation)), body->positionY, body->positionZ + movementRate*glm::cos(glm::radians(xzOrientation)))*rotate(glm::radians(xzOrientation), 0, 1, 0)*scale(1.5, 0.5, 2);
		

			body->setPosition(body->positionX + movementRate*glm::sin(glm::radians(xzOrientation)), body->positionY, body->positionZ + movementRate*glm::cos(glm::radians(xzOrientation)));
			turret->setPosition(turret->positionX + movementRate*glm::sin(glm::radians(xzOrientation)), turret->positionY, turret->positionZ + movementRate*glm::cos(glm::radians(xzOrientation)));
			cannon->setPosition(cannon->positionX + movementRate*glm::sin(glm::radians(xzOrientation)), cannon->positionY + 0.675, cannon->positionZ + movementRate*glm::cos(glm::radians(xzOrientation)));
			moveTurret(upOrientation, rightOrientation);
		
		}
		void moveTurret(GLfloat upAngle,GLfloat rightAngle) {
			upOrientation = upAngle;
			rightOrientation = rightAngle;
			turret->transform = translate(turret->positionX, turret->positionY, turret->positionZ)*rotate(glm::radians(-rightOrientation), 0, 1, 0)*scale(0.5, 0.75, 1);
			cannon->transform = translate(turret->positionX + glm::sin(glm::radians(rightOrientation)) + 0.01*(glm::sin(glm::radians(xzOrientation))), turret->positionY +0.325 - glm::sin(glm::radians(upOrientation)), turret->positionZ - (glm::cos(glm::radians(-rightOrientation))))*rotate(glm::radians(-rightOrientation), 0, 1, 0)*rotate(-glm::radians(upOrientation), 1, 0, 0)*scale(0.1, 0.1, 1);
			cannon->setPosition(turret->positionX + glm::sin(glm::radians(rightOrientation)) + 0.01*(glm::sin(glm::radians(xzOrientation))), turret->positionY + 0.325 - glm::sin(glm::radians(upOrientation)), turret->positionZ - (glm::cos(glm::radians(-rightOrientation))));
		}
		void removeHealth(double dmg) {
			health -= dmg;
		}
		Projectile Tank::shoot(ModelAsset gBall){
			std::chrono::steady_clock::time_point currentAttack = std::chrono::steady_clock::now();
			double duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentAttack - lastAttack).count();
			if (duration > MIN_ATTACK_INTERVAL) {
				std::cout << "BANG" << std::endl;
				lastAttack = currentAttack;
				return Projectile(cannon->positionX, cannon->positionY, cannon->positionZ, gBall, rightOrientation, upOrientation);
			}	
			return Projectile();
		}
		GLfloat getXZOrientation() { return xzOrientation;}
		GLfloat getUpOrientation() { return upOrientation; }
		GLfloat getRightOrientation() { return rightOrientation; }
		double getHealth() { return health; }
		
	private:
		ModelInstance* cannon, *turret, *body;
		GLfloat x, y, z;
		GLfloat xzOrientation=0;
		GLfloat upOrientation = 0, rightOrientation = 0;
		double health;
		std::chrono::steady_clock::time_point lastAttack;
		
	};
}
#endif