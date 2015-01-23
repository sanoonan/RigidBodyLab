#include "RigidBody.h"

using namespace std;

RigidBody :: RigidBody()
{
	position = orientation = velocity = ang_velocity = centre_of_mass = glm::vec3(0.0f);
	translation_mat = glm::mat4();
	inertial_tensor = rotation_mat = glm::mat3();
	mass = 0;
}

RigidBody :: RigidBody(Mesh _mesh)
{
	position = orientation = velocity = ang_velocity = centre_of_mass = glm::vec3(0.0f);
	translation_mat  = glm::mat4();
	inertial_tensor = rotation_mat = glm::mat3();
	mesh = _mesh;
	mass = 1;
}

void RigidBody :: load_mesh()
{
	mesh.load_mesh(vertices);

	inertial_tensor = calcInertialTensorBox();
}

void RigidBody :: draw(int m_loc)
{	
	glm::mat4 m_mat = translation_mat * glm::mat4(rotation_mat);

	mesh.draw(m_mat, m_loc);
}

void RigidBody :: addTBar(TwBar *bar)
{
	TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &position, "");
	TwAddVarRO(bar, "Orientation", TW_TYPE_DIR3F, &orientation, "");
	TwAddVarRW(bar, "Velocity", TW_TYPE_DIR3F, &velocity, "");
	TwAddVarRW(bar, "Angular Velocity", TW_TYPE_DIR3F, &ang_velocity, "");
}

void RigidBody :: update(float dt)
{
	updateTranslation(dt);
	updateRotation(dt);
	cullInstances(dt);
}

void RigidBody :: updateTranslation(float dt)
{
	glm::vec3 total_force = calcTotalForce();

	velocity += total_force * dt / mass;

	position += dt * velocity;

	translation_mat = glm::translate(glm::mat4(), position);
}

void RigidBody :: updateRotation(float dt)
{
	glm::vec3 total_torque = calcTotalTorque();

	glm::mat3 moment_inertia = calcMomentInertia();
	glm::mat3 inv_moment = glm::inverse(moment_inertia);

	ang_velocity += inv_moment * total_torque * dt;

	glm::mat3 ang_vel_mat = makeAngVelMat();

	rotation_mat += ang_vel_mat * rotation_mat * dt;	

	rotation_mat = glm::orthonormalize(rotation_mat);

	orientation = getOrientationFromRotMat(rotation_mat);
}

glm::mat3 RigidBody :: makeAngVelMat()
{
	glm::mat3 av;

	av[0][1] = -ang_velocity.z;
	av[0][2] = ang_velocity.y;

	av[1][0] = ang_velocity.z;
	av[1][2] = -ang_velocity.x;

	av[2][0] = -ang_velocity.y;
	av[2][1] = ang_velocity.x;

	return av;
}




glm::vec3 RigidBody :: getOrientationFromRotMat(glm::mat3 mat)
{
	float x, y, z;
	glm::vec3 vec;

	x = atan2(mat[2][1], mat[2][2]);
	y = atan2(-mat[2][0], sqrt(mat[2][1]*mat[2][1] + mat[2][2]*mat[2][2]));
	z = atan2(mat[1][0], mat[0][0]);

	x = glm::degrees(x);
	y = glm::degrees(y);
	z = glm::degrees(z);

	while(x < 0)
		x += 360;

	while(y < 0)
		y += 360;

	while(z < 0)
		z += 360;

	vec = glm::vec3(x, y, z);
	return vec;

}

glm::mat3 RigidBody :: calcInertialTensorBox()
{
	float width, height, depth;
	float min_x, min_y, min_z, max_x, max_y, max_z;

	min_x = max_x = vertices[0].x;
	min_y = max_y = vertices[0].y;
	min_z = max_z = vertices[0].z;

	for(int i=1; i<vertices.size(); i++)
	{
		if(vertices[i].x < min_x)
			min_x = vertices[i].x;
		if(vertices[i].x > max_x)
			max_x = vertices[i].x;

		if(vertices[i].y < min_y)
			min_y = vertices[i].y;
		if(vertices[i].y > max_y)
			max_y = vertices[i].y;

		if(vertices[i].z < min_z)
			min_z = vertices[i].z;
		if(vertices[i].z > max_z)
			max_z = vertices[i].z;
	}

	width = max_x - min_x;
	height = max_y - min_y;
	depth = max_z - min_z;

	glm::mat3 it;
	float a, b, c;

	a = (mass*(height*height + depth*depth))/12;
	b = (mass*(width*width + depth*depth))/12;
	c = (mass*(width*width + height*height))/12;

	it[0][0] = a;
	it[1][1] = b;
	it[2][2] = c;

	return it;
}

glm::mat3 RigidBody :: calcMomentInertia()
{
	glm::mat3 rt = glm::transpose(rotation_mat);
	glm::mat3 mi = rotation_mat * inertial_tensor * rt;

	return mi;
}


void RigidBody :: affectedByForce(Effector effector)
{
	force_instance fi;
	fi.force = effector.force;
	fi.time_left = effector.time;

	fi.torque = glm::cross((effector.position - centre_of_mass), effector.force);

	


	int fi_list_size = force_instances.size();
	force_instances.resize(fi_list_size + 1);
	force_instances[fi_list_size] = fi;
}

glm::vec3 RigidBody :: calcTotalForce()
{
	glm::vec3 t = glm::vec3(0.0f);

	int size = force_instances.size();

	for(int i=0; i<size; i++)
		t += force_instances[i].force;

	return t;
}

glm::vec3 RigidBody :: calcTotalTorque()
{
	glm::vec3 t = glm::vec3(0.0f);

	int size = force_instances.size();

	for(int i=0; i<size; i++)
		t += force_instances[i].torque;

	return t;

}

void RigidBody :: cullInstances(float dt)
{
	int size = force_instances.size();

	for(int i=0; i<size; i++)
	{
		force_instances[i].time_left -= dt;
		if(force_instances[i].time_left <= 0)
		{
			removeInstance(i);
			size--;
		}
	}
}

void RigidBody :: removeInstance(int num)
{
	int size = force_instances.size();

	if(num >= size)
		return;

	for(int i=num; i<size-1; i++)
		force_instances[i] = force_instances[i+1];

	force_instances.resize(size-1);
}