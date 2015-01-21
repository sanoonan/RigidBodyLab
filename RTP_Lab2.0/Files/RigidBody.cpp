#include "RigidBody.h"

using namespace std;

RigidBody :: RigidBody()
{
	position = orientation = velocity = ang_velocity = glm::vec3(0.0f);
	translation_mat = rotation_mat = glm::mat4();
	mass = 0;
}

RigidBody :: RigidBody(Mesh _mesh)
{
	position = orientation = velocity = ang_velocity = glm::vec3(0.0f);
	translation_mat = rotation_mat = glm::mat4();
	mesh = _mesh;
	mass = 1;
}

void RigidBody :: draw(int m_loc)
{	
	glm::mat4 m_mat = translation_mat * rotation_mat;

	mesh.draw(m_mat, m_loc);
}

void RigidBody :: addTBar(TwBar *bar)
{
	TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &position, "");
	TwAddVarRW(bar, "Orientation", TW_TYPE_DIR3F, &orientation, " opened=true ");
	TwAddVarRW(bar, "Velocity", TW_TYPE_DIR3F, &velocity, "");
	TwAddVarRW(bar, "Angular Velocity", TW_TYPE_DIR3F, &ang_velocity, " opened=true ");
}

void RigidBody :: update(float dt)
{
	updateTranslation(dt);
	updateRotation(dt);
}

void RigidBody :: updateTranslation(float dt)
{
	position += dt * velocity;
	translation_mat = glm::translate(glm::mat4(), position);
}

void RigidBody :: updateRotation(float dt)
{
//	rotation_mat = glm::mat4();

//	rotation_mat = glm::rotate(rotation_mat, orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
//	rotation_mat = glm::rotate(rotation_mat, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
//	rotation_mat = glm::rotate(rotation_mat, orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 ang_vel_mat = makeAngVelMat();

	rotation_mat += ang_vel_mat * rotation_mat * dt;
}

glm::mat4 RigidBody :: makeAngVelMat()
{
	glm::mat4 av;

	av[0][1] = -ang_velocity.z;
	av[0][2] = ang_velocity.y;

	av[1][0] = ang_velocity.z;
	av[1][2] = -ang_velocity.x;

	av[2][0] = -ang_velocity.y;
	av[2][1] = ang_velocity.x;

	return av;
}


