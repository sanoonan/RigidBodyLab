#pragma once

#define _CRT_SECURE_NO_DEPRECATE
//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>


// Assimp includes

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.
#include <string>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4, glm::ivec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtx/orthonormalize.hpp>

#include "Mesh.h"
#include "Effector.h"

#include "AntTweakBar.h"

class RigidBody
{
public:

	glm::vec3 position;
	glm::vec3 orientation;

	glm::vec3 velocity;
	glm::vec3 ang_velocity;


	glm::mat4 translation_mat;
	glm::mat3 rotation_mat;

	struct force_instance
	{
		glm::vec3 force, torque;
		float time_left;

		force_instance() {force = torque = glm::vec3(0.0f); time_left = 0;}
	};

	std::vector<force_instance> force_instances;



	glm::mat3 inertial_tensor;
	glm::vec3 centre_of_mass;

	std::vector<glm::vec3> vertices;

	float mass;


	Mesh mesh;

	RigidBody();
	RigidBody(Mesh _mesh);

	void load_mesh();
	void draw(int m_loc);

	void addTBar(TwBar *bar);

	void update(float dt);

	void updateTranslation(float dt);
	void updateRotation(float dt);
	glm::vec3 getOrientationFromRotMat(glm::mat3 mat);

	
	
	glm::mat3 makeAngVelMat();

	glm::mat3 calcInertialTensorBox();
	glm::mat3 calcMomentInertia();

	void affectedByForce(Effector effector);

	glm::vec3 calcTotalForce();
	glm::vec3 calcTotalTorque();
	void cullInstances(float dt);

	void removeInstance(int num);
};

