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
#include <stdlib.h>
#include "assert.h"

// Header includes

#include "RigidBody.h"
#include "Effector.h"
#include "Camera.h"

#include "AntTweakBar.h"

using namespace std;

int width = 1200;
int height = 900;

#define V_SHADER_NOTEXTURE "../Shaders/noTextureVertexShader.txt"
#define F_SHADER_NOTEXTURE "../Shaders/noTextureFragmentShader.txt"
#define MESH_CUBE "Meshes/cube.dae"

int oldTime = 0;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));


GLuint shaderProgramID;

Mesh cube_mesh(MESH_CUBE);

RigidBody cube(cube_mesh);
Effector effector;

glm::mat4 proj_mat, view_mat;

#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {   
    FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

    if ( fp == NULL ) { return NULL; }

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    char* buf = new char[size + 1];
    fread(buf, 1, size, fp);
    buf[size] = '\0';

    fclose(fp);

    return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
       // exit(0);
    }
	const char* pShaderSource = readShaderSource( pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
	// check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        //exit(1);
    }
	// Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertex_shader, const char* fragment_shader)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
    GLuint shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
      //  exit(1);
    }

	// Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgramID, vertex_shader, GL_VERTEX_SHADER);
    AddShader(shaderProgramID, fragment_shader, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        //exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        //exit(1);
    }
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion 

#pragma region TWEAK BAR STUFF

void TW_CALL poke(void *)
{
	cube.affectedByForce(effector);
}

void init_tweak()
{
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(width, height);

	TwBar *bar;
	bar = TwNewBar("Rigid Body");

	cube.addTBar(bar);
	effector.addTBar(bar);
	TwAddButton(bar, "Poke", poke, NULL, "");
	camera.addTBar(bar);
}

void draw_tweak()
{
	TwDraw();
}



#pragma endregion 


void init()
{
	shaderProgramID = CompileShaders(V_SHADER_NOTEXTURE, F_SHADER_NOTEXTURE);

	cube.load_mesh();

	init_tweak();

	camera.move_speed = 0.1f;

}

void display()
{
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);

	int model_mat_location = glGetUniformLocation (shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation (shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation (shaderProgramID, "proj");

	proj_mat = glm::perspective(45.0f, (float)width/(float)height, 0.1f, 200.0f);
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, glm::value_ptr(proj_mat));

	
	view_mat = camera.getRotationMat();
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, glm::value_ptr(view_mat));

	cube.draw(model_mat_location);


	draw_tweak();

	glutSwapBuffers();
}

void updateScene()
{
	int time = glutGet(GLUT_ELAPSED_TIME);
	int delta_time = time-oldTime;
	oldTime = time;

	//time since last frame in seconds
	double elapsed_seconds = (double)delta_time/1000;

	cube.update(elapsed_seconds);
	cube.transformVertices(proj_mat, view_mat);

	glutPostRedisplay();
}


void getMouseRayPoints(int x, int y, glm::vec3 &p1, glm::vec3 &p2, RigidBody object)
{
	// This function will find 2 points in world space that are on the line into the screen defined by screen-space( ie. window-space ) point (x,y)
	double mvmatrix[16];
	double projmatrix[16];
	int viewport[4];
	double dX, dY, dZ, dClickY; // glUnProject uses doubles, but I'm using floats for these 3D vectors

	glm::mat4 mv_mat = view_mat;
	
	const float *pSource = (const float*) glm::value_ptr(mv_mat);
	for(int i=0; i<16; i++)
		mvmatrix[i] = pSource[i];

	pSource = (const float*) glm::value_ptr(proj_mat);
	for(int i=0; i<16; i++)
		projmatrix[i] = pSource[i];

	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = width;
	viewport[3] = height;
	
//	glGetIntegerv(GL_VIEWPORT, viewport);	
//	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
//	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	dClickY = double (height - y); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top
	
	gluUnProject ((double) x, dClickY, 0.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
	p1 = glm::vec3 ( (float) dX, (float) dY, (float) dZ );
	gluUnProject ((double) x, dClickY, 1.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
	p2 = glm::vec3 ( (float) dX, (float) dY, (float) dZ );
}


bool checkRayTriangleIntersection(glm::vec3 TP1, glm::vec3 TP2, glm::vec3 TP3, glm::vec3 LP1, glm::vec3 LP2, glm::vec3 &HitPos)
{
	glm::vec3 Normal, IntersectPos;

	// Find Triangle Normal
	Normal = glm::cross( TP2 - TP1, TP3 - TP1 );
	glm::normalize(Normal); // not really needed

	// Find distance from LP1 and LP2 to the plane defined by the triangle
	float Dist1 = glm::dot(LP1-TP1, Normal);;
	float Dist2 = glm::dot(LP2-TP1, Normal);

	float lol = Dist1 * Dist2;

	if ( (Dist1 * Dist2) >= 0.0f) 
		return false;	// line doesn't cross the triangle.
	if ( Dist1 == Dist2) 
		return false;	// line and plane are parallel

	// Find point on the line that intersects with the plane
	IntersectPos = LP1 + (LP2-LP1) * ( -Dist1/(Dist2-Dist1) );

	// Find if the interesection point lies inside the triangle by testing it against all edges
	glm::vec3 vTest;
	
	vTest = glm::cross( Normal, TP2-TP1 );
	if ( glm::dot( vTest, IntersectPos-TP1) < 0.0f )
		return false;
	
	vTest = glm::cross( Normal, TP3-TP2 );
	if ( glm::dot( vTest, IntersectPos-TP2) < 0.0f )
		return false;
	
	vTest = glm::cross( Normal, TP1-TP3 );
	if ( glm::dot( vTest, IntersectPos-TP1) < 0.0f )
		return false;

	HitPos = IntersectPos;
	return true;
}


void mousePress(int button, int state, int x, int y)
{
	if(!TwEventMouseButtonGLUT(button, state, x, y))
	{
		if((button == GLUT_LEFT_BUTTON)&&(state==GLUT_DOWN))
		{
			cout << "click" << endl;

			glm::vec3 p1, p2;
			getMouseRayPoints(x, y, p1, p2, cube);
			bool hit = false;
			float min_hit_dist;
			glm::vec3 min_hit_pos;


			for(int i=0; i < cube.transformed_vertices.size(); i+=3)
			{
				glm::vec3 hit_position;
				if(checkRayTriangleIntersection(cube.transformed_vertices[i], cube.transformed_vertices[i+1], cube.transformed_vertices[i+2], p1, p2, hit_position))
				{
					float hit_dist = glm::distance(camera.position, hit_position);

					if(!hit)
					{
						hit = true;
						cout << "hit" << endl;
						min_hit_pos = hit_position;
						min_hit_dist = hit_dist;
					}
					else
					{
						if(hit_dist < min_hit_dist)
						{
							min_hit_dist = hit_dist;
							min_hit_pos = hit_position;
						}
					}
				}
			}

			if(hit)
			{
				effector.force_dir = min_hit_pos - camera.position;
				effector.position = min_hit_pos;
				cube.affectedByForce(effector);
			}

	
		
		}
	}
}

void specialPress(int key, int x, int y)
{
	if(!TwEventSpecialGLUT(key, x, y))
	{
		switch (key)
		{
		//left arrow
		case 100 :
			camera.moveLeft();
		break;

		//up arrow
		case 101 :	
			camera.moveForwards();
		break;
	
		//right arrow
		case 102 :
			camera.moveRight();
		break;

		//down arrow
		case 103:
			camera.moveBackwards();
		break;

	
		}
	}

glutPostRedisplay(); 
}


void keyPress(unsigned char key, int x, int y)
{
	if(!TwEventKeyboardGLUT(key, x, y))
	{
		switch (key)
		{
		//left
		case 'a' :
			camera.turnLeft();
		break;

		//up
		case 'w' :	
			camera.lookUp();
		break;
	
		//right
		case 'd' :
			camera.turnRight();
		break;

		//down
		case 's':
			camera.lookDown();
		break;

	
		}
	}

glutPostRedisplay(); 
}
	

int main(int argc, char** argv)
{

	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Rigid Body");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);


	 glutMouseFunc(mousePress);
	 glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	 glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); // same as MouseMotion
	 glutKeyboardFunc(keyPress);
	 glutSpecialFunc(specialPress);
 
	  // send the ''glutGetModifers'' function pointer to AntTweakBar
	 TwGLUTModifiersFunc(glutGetModifiers);
 


	 // A call to glewInit() must be done after glut is initialized!
    GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
    return 0;
}