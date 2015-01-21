#include "Camera.h"

using namespace std;

Camera :: Camera()
{
	position = glm::vec3(0.0f);
}

Camera :: Camera(glm::vec3 _position)
{
	position = _position;
}

glm::mat4 Camera :: getRotationMat(glm::vec3 focus)
{
	glm::mat4 ret = glm::lookAt(position, focus, glm::vec3(0.0f, 1.0f, 0.0f));
	return ret;
}