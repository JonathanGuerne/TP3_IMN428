/******************************************************
TP3

STUDENTS:

AUTHORS: Pierre-Marc Jodoin, Louis-Philippe Ledoux

DATE : Winter 2016
*******************************************************/

#define GLUT_DISABLE_ATEXIT_HACK

#define NDEBUG

#include <windows.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <GL/gl.h>

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <vector>

#include <iostream>
#include <fstream>
#include <sstream>

#include <math.h>

//GLM Includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "structures.h"

using namespace std;


/******************************\
*****   Global variables   *****
\******************************/
MouseEvent	lastMouseEvt;			/* Last mouse event */
CamInfo		gCam;	                /* Camera parameters */

Material	materials[2];	        /* 0 = plane, 1 = revolution object */
Light		lights[2]; 				/* 0 = white, 1 = blue */
Light*		currentLight = lights;/* Current light, the one selected with the mouse middle button */

// Silhouette parameters : the silhouette contains 17 2D points 
#define		NB_PTS_ON_SILHOUETTE 17
glm::vec2	silhouettePointArray[NB_PTS_ON_SILHOUETTE];
int 		objectResolution = 7;	/* the number of times the silhouette is rotated. */

// On/off binary variables
bool		displayNormals = false;
bool		displayRevolutionObject = false;
bool        isLightEnabled = true;	/* Lighting is disabled when = false */

// Array and buffer Objects. vao:vertex array object, vbo: vertex buffer object, nbo: normal buffer object
unsigned int vaoPlaneID;
unsigned int vboPlaneID;
unsigned int nboPlaneID;

unsigned int vaoRevolutionID;
unsigned int vboRevolutionID;
unsigned int nboRevolutionID;

unsigned int vaoLightID;
unsigned int vboLightID;

unsigned int vaoNormalsID;
unsigned int vboNormalsID;

// Camera matrices
glm::mat4	projectionMatrix;
glm::mat4	viewMatrix;

// Shader
Shader*		shader;

/**********************\
***** Prototypes *****
\**********************/
// function for creating vertices, copying it on the GPU and drawing objects
void updatePlaneMesh();
void updateRevolutionObjectMesh();
void updateNormalLines();

void drawPlane();
void drawRevolutionObject();
void drawLight(int idx);
std::vector<glm::vec3> getSphereVertices(float radius, int slices, int stacks);

// Initialization functions
void initObjects();
void initGPUBuffers(void);
void initLights();
void initMaterials();

void setCameraMatrices();

// Callbacks
void keyboard(unsigned char key, int x, int y);
void mouseButton(int button, int state, int x, int y);
void mouseMove(int x, int y);
void display(void);


/*****************************************************
NAME: deg2rad and rad2deg

DESCRIPTION: Switche degrees to radians, and the opposite

*****************************************************/
float deg2rad(float deg)
{
	return	0.01745329251994329547437168059786927187815f * deg;
}

float rad2deg(float rad)
{
	return	57.2957795130823228646477218717336654663085f * rad;
}

/*****************************************************
NAME: initMaterials

DESCRIPTION: initialize material parameters.
*****************************************************/
void initMaterials()
{
	// Gold Plane 
	materials[0].ambient[0] = 0.33f; materials[0].ambient[1] = 0.22f; materials[0].ambient[2] = 0.03f; materials[0].ambient[3] = 1.f;
	materials[0].diffuse[0] = 0.7f; materials[0].diffuse[1] = 0.5f; materials[0].diffuse[2] = 0.1f; materials[0].diffuse[3] = 1.f;
	materials[0].specular[0] = 1.f; materials[0].specular[1] = 1.f; materials[0].specular[2] = 1.f; materials[0].specular[3] = 1.f;
	materials[0].shininess = 127.f;

	// Red Revolution object
	materials[1].ambient[0] = 0.5f; materials[1].ambient[1] = 0.1f; materials[1].ambient[2] = 0.f; materials[1].ambient[3] = 1.f;
	materials[1].diffuse[0] = 1.f; materials[1].diffuse[1] = 0.2f; materials[1].diffuse[2] = 0.f; materials[1].diffuse[3] = 1.f;
	materials[1].specular[0] = 0.5f; materials[1].specular[1] = 0.3f; materials[1].specular[2] = 0.f; materials[1].specular[3] = 1.f;
	materials[1].shininess = 3.f;
}

/*****************************************************
NAME: initObjects

DESCRIPTION: initialize the objects, and the silhouette for the revolution object.
	This means creating the GPU vertex arrays and vertex buffers.

	For the lights, since their mesh is constant, we shall also create its
	vertices and copy it in the GPU.
*****************************************************/
void initObjects()
{
	silhouettePointArray[0].x = 2.f;	silhouettePointArray[0].y = -100.f;
	silhouettePointArray[1].x = 43.f;	silhouettePointArray[1].y = -100.f;
	silhouettePointArray[2].x = 86.f;	silhouettePointArray[2].y = -100.f;
	silhouettePointArray[3].x = 61.f;	silhouettePointArray[3].y = -91.f;
	silhouettePointArray[4].x = 40.f;	silhouettePointArray[4].y = -88.f;
	silhouettePointArray[5].x = 17.f;	silhouettePointArray[5].y = -83.f;
	silhouettePointArray[6].x = 25.f;	silhouettePointArray[6].y = -54.f;
	silhouettePointArray[7].x = 24.f;	silhouettePointArray[7].y = -30.f;
	silhouettePointArray[8].x = 22.f;	silhouettePointArray[8].y = -5.f;
	silhouettePointArray[9].x = 14.f;	silhouettePointArray[9].y = 21.f;
	silhouettePointArray[10].x = 11.f;	silhouettePointArray[10].y = 54.f;
	silhouettePointArray[11].x = 25.f;	silhouettePointArray[11].y = 60.f;
	silhouettePointArray[12].x = 40.f;	silhouettePointArray[12].y = 65.f;
	silhouettePointArray[13].x = 54.f;	silhouettePointArray[13].y = 67.f;
	silhouettePointArray[14].x = 70.f;	silhouettePointArray[14].y = 75.f;
	silhouettePointArray[15].x = 80.f;	silhouettePointArray[15].y = 84.f;
	silhouettePointArray[16].x = 85.f;	silhouettePointArray[16].y = 95.f;

	initGPUBuffers();
	initLights();
}

/*****************************************************
NAME: initGPUBuffers

DESCRIPTION: Create the vertex array object and buffer object for each 3D object
*****************************************************/
void initGPUBuffers(void)
{
	// Sphere for the 3D lights
	glGenVertexArrays(1, &vaoLightID);
	glGenBuffers(1, &vboLightID);

	//Revolution objects
	glGenVertexArrays(1, &vaoRevolutionID);
	glGenBuffers(1, &vboRevolutionID);
	glGenBuffers(1, &nboRevolutionID);

	//Plane
	glGenVertexArrays(1, &vaoPlaneID);
	glGenBuffers(1, &vboPlaneID);
	glGenBuffers(1, &nboPlaneID);

	//Normals
	glLineWidth(1.f);

	glGenVertexArrays(1, &vaoNormalsID);
	glGenBuffers(1, &vboNormalsID);

}

/*****************************************************
NAME: initLights

DESCRIPTION: This function does 3 things :

1- Set the light properties
2- create the vertices of a sphere
3- copy the vertices on the GPU
*****************************************************/
void initLights()
{
	//	Set Light properties
	lights[0].ambient[0] = 0.7f; lights[0].ambient[1] = 0.7f; lights[0].ambient[2] = 0.7f; lights[0].ambient[3] = 1.f;
	lights[0].diffuse[0] = 0.7f; lights[0].diffuse[1] = 0.7f; lights[0].diffuse[2] = 0.7f; lights[0].diffuse[3] = 1.f;
	lights[0].specular[0] = 0.7f; lights[0].specular[1] = 0.7f; lights[0].specular[2] = 0.7f; lights[0].specular[3] = 1.f;
	lights[0].position[0] = 70.f; lights[0].position[1] = -70.f; lights[0].position[2] = 70.f; lights[0].position[3] = 1.f;

	lights[0].Kc = 0.f;
	lights[0].Kl = 0.01f;
	lights[0].Kq = 0.f;
	lights[0].on = true;

	lights[1].ambient[0] = 0.f; lights[1].ambient[1] = 0.f; lights[1].ambient[2] = 1.f; lights[1].ambient[3] = 1.f;
	lights[1].diffuse[0] = 0.f; lights[1].diffuse[1] = 0.f; lights[1].diffuse[2] = 1.f; lights[1].diffuse[3] = 1.f;
	lights[1].specular[0] = 0.f; lights[1].specular[1] = 0.f; lights[1].specular[2] = 1.f; lights[1].specular[3] = 1.f;
	lights[1].position[0] = -10.f; lights[1].position[1] = -90.f; lights[1].position[2] = -10.f; lights[1].position[3] = 1.f;

	lights[1].Kc = 1.f;
	lights[1].Kl = 0.f;
	lights[1].Kq = 0.f;
	lights[1].on = true;

	//	Create light vertices : 3D sphere
	std::vector<glm::vec3> vertices = getSphereVertices(5, 10, 10);

	// Bind the Vertex Array Object so we can use it 
	glBindVertexArray(vaoLightID);

	glBindBuffer(GL_ARRAY_BUFFER, vboLightID); // Bind the Vertex Buffer Object 
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW); // Set the size and data of our VBO  

	int in_PositionLocation = glGetAttribLocation(shader->id(), "in_Position");
	glEnableVertexAttribArray(in_PositionLocation);
	glVertexAttribPointer(in_PositionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0); // Set up the vertex attributes pointer
}

/*****************************************************
NAME: copyLightParametersOnGPU

DESCRIPTION: copy the parameters of both lights on the GPU
*****************************************************/
void copyLightParametersOnGPU(void)
{
	shader->bind(); // activate shader

	int lightPosLocation = glGetUniformLocation(shader->id(), "lightPosition");
	int lightLaLocation = glGetUniformLocation(shader->id(), "lightLa");
	int lightLdLocation = glGetUniformLocation(shader->id(), "lightLd");
	int lightLsLocation = glGetUniformLocation(shader->id(), "lightLs");
	int lightKcLocation = glGetUniformLocation(shader->id(), "lightKc");
	int lightKlLocation = glGetUniformLocation(shader->id(), "lightKl");
	int lightKqLocation = glGetUniformLocation(shader->id(), "lightKq");

	int lightPosLocation2 = glGetUniformLocation(shader->id(), "lightPosition2");
	int lightLaLocation2 = glGetUniformLocation(shader->id(), "lightLa2");
	int lightLdLocation2 = glGetUniformLocation(shader->id(), "lightLd2");
	int lightLsLocation2 = glGetUniformLocation(shader->id(), "lightLs2");
	int lightKcLocation2 = glGetUniformLocation(shader->id(), "lightKc2");
	int lightKlLocation2 = glGetUniformLocation(shader->id(), "lightKl2");
	int lightKqLocation2 = glGetUniformLocation(shader->id(), "lightKq2");

	glUniform4f(lightPosLocation, lights[0].position[0], lights[0].position[1], lights[0].position[2], isLightEnabled ? 1.f : 0.f);
	glUniform3f(lightLaLocation, lights[0].ambient[0], lights[0].ambient[1], lights[0].ambient[2]);
	glUniform3f(lightLdLocation, lights[0].diffuse[0], lights[0].diffuse[1], lights[0].diffuse[2]);
	glUniform3f(lightLsLocation, lights[0].specular[0], lights[0].specular[1], lights[0].specular[2]);
	glUniform1f(lightKcLocation, lights[0].Kc);
	glUniform1f(lightKlLocation, lights[0].Kl);
	glUniform1f(lightKqLocation, lights[0].Kq);

	glUniform4f(lightPosLocation2, lights[1].position[0], lights[1].position[1], lights[1].position[2], isLightEnabled ? 1.f : 0.f);
	glUniform3f(lightLaLocation2, lights[1].ambient[0], lights[1].ambient[1], lights[1].ambient[2]);
	glUniform3f(lightLdLocation2, lights[1].diffuse[0], lights[1].diffuse[1], lights[1].diffuse[2]);
	glUniform3f(lightLsLocation2, lights[1].specular[0], lights[1].specular[1], lights[1].specular[2]);
	glUniform1f(lightKcLocation2, lights[1].Kc);
	glUniform1f(lightKlLocation2, lights[1].Kl);
	glUniform1f(lightKqLocation2, lights[1].Kq);

	shader->unbind(); // deactivate shader
}


glm::vec3 getNormalForVertex(int silArrayIndex, float currentAngle, float tetha) {

	std::vector<glm::vec3> vertexs;

	glm::vec3 vertex;
	glm::vec3 center;

	glm::vec3 vectrices[9];
	/**************
	1-----2-----3
	|     |     |
	|     |     |
	4-----0-----5
	|     |     |
	|     |     |
	6-----7-----8
	where 0 is the point where we want to calculate the normal
	**************/


	vectrices[0].x = (silhouettePointArray[silArrayIndex].x)*cos(currentAngle);
	vectrices[0].y = silhouettePointArray[(silArrayIndex)].y;
	vectrices[0].z = -(silhouettePointArray[(silArrayIndex)].x)*sin(currentAngle);

	int index = 1;

	for (int s = silArrayIndex - 1; s <= silArrayIndex + 1; s++) {
		for (float a = currentAngle - tetha; a <= currentAngle + tetha; a += tetha) {
			if (a != currentAngle || s != silArrayIndex) {
				if (s >= 0 && s < NB_PTS_ON_SILHOUETTE) {

					vectrices[index].x = (silhouettePointArray[s].x)*cos(a);
					vectrices[index].y = silhouettePointArray[(s)].y;
					vectrices[index].z = -(silhouettePointArray[(s)].x)*sin(a);

				}
				index++;
			}
		}
	}

	std::vector<glm::vec3> normals;
	glm::vec3 normal;

	glm::vec3 vec1;
	glm::vec3 vec2;


	vec1 = glm::normalize(vectrices[1] - vectrices[0]);

	vec2 = glm::normalize(vectrices[2] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[3] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[4] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[5] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[6] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[7] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));


	vec1 = glm::normalize(vectrices[2] - vectrices[0]);

	vec2 = glm::normalize(vectrices[3] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[4] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[5] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[6] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[8] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));


	vec1 = glm::normalize(vectrices[3] - vectrices[0]);

	vec2 = glm::normalize(vectrices[4] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[5] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[7] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[8] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));


	vec1 = glm::normalize(vectrices[4] - vectrices[0]);

	vec2 = glm::normalize(vectrices[6] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[7] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[8] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));


	vec1 = glm::normalize(vectrices[5] - vectrices[0]);

	vec2 = glm::normalize(vectrices[6] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[7] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));

	vec2 = glm::normalize(vectrices[8] - vectrices[0]);
	normals.push_back(glm::cross(vec1, vec2));


	vec1 = glm::normalize(vectrices[6] - vectrices[0]);

	vec2 = glm::normalize(vectrices[7] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	vec2 = glm::normalize(vectrices[8] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));


	vec1 = glm::normalize(vectrices[7] - vectrices[0]);

	vec2 = glm::normalize(vectrices[8] - vectrices[0]);
	normals.push_back(glm::cross(vec2, vec1));

	glm::vec3 output;
	int count = 0;

	for (int i = 0; i < normals.size(); i++) {

		if (glm::length(normals[i]) != 0) {
			count++;
			output = normals[i] + output;
		}

	}

	output.x = output.x / count;
	output.y = output.y / count;
	output.z = output.z / count;

	return glm::normalize(output);

}

/*****************************************************
NAME: updateNormalLines

DESCRIPTION: create a series of lines each made of 2 vertices.
	then copy it on the GPU.  These lines are used to
	illustrate the normals at each vertex of the revolution
	object.
*****************************************************/
void updateNormalLines()
{
	// AJOUTER CODE ICI
	std::vector<glm::vec3> vertices;
	glm::vec3 vertex;
	glm::vec3 normal;

	float factor = 20.0;

	float tetha = deg2rad(360) / objectResolution;

	for (int n = 0; n < NB_PTS_ON_SILHOUETTE - 1; n++) {

		float currentAngle = 0.0f;
		std::vector<glm::vec3> temp;

		for (int k = 0; k < objectResolution; k++) {

			vertex.x = (silhouettePointArray[(n)].x)*cos(currentAngle);
			vertex.y = silhouettePointArray[(n)].y;
			vertex.z = -(silhouettePointArray[(n)].x)*sin(currentAngle);

			vertices.push_back(vertex);

			normal = getNormalForVertex(n, currentAngle, tetha);

			normal.x *= factor;
			normal.y *= factor;
			normal.z *= factor;


			vertices.push_back(vertex + normal);

			currentAngle += tetha;

		}

	}

	glBindVertexArray(vaoNormalsID);

	glBindBuffer(GL_ARRAY_BUFFER, vboNormalsID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	int in_PositionLocation = glGetAttribLocation(shader->id(), "in_Position");
	glEnableVertexAttribArray(in_PositionLocation);
	glVertexAttribPointer(in_PositionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
}



/*****************************************************
NAME: drawNormals

DESCRIPTION: draw the normal at each vertex of the revolution object.
Somewhat similar to "drawLight"

*****************************************************/
void drawNormals()
{
	shader->bind(); // activate shader

	glm::mat4 normalMatrix = glm::mat4(1.f);

	// Get the locations of uniform variables
	int projectionMatrixLocation = glGetUniformLocation(shader->id(), "projectionMatrix");
	int viewMatrixLocation = glGetUniformLocation(shader->id(), "viewMatrix");
	int modelMatrixLocation = glGetUniformLocation(shader->id(), "modelMatrix");
	int materialKdLocation = glGetUniformLocation(shader->id(), "materialKd");

	// Copy data to the GPU
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &normalMatrix[0][0]);
	glUniform3f(materialKdLocation, 1.0f, 1.0f, 1.0f);

	// Draw object
	glBindVertexArray(vaoNormalsID);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormalsID);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_LINES, 0, (NB_PTS_ON_SILHOUETTE - 1) * objectResolution * 2);

	glBindVertexArray(0);

	shader->unbind(); // deactivate shader
}



/*****************************************************
NAME: updatePlaneMesh

DESCRIPTION: create plane vertices and copy it on the GPU.   The number
of vertices on the plane is defined by the global variable "objectResolution".
The plane spans from -100 to 100 along the X and Z axis and is located
at Y=-100;

*****************************************************/
void updatePlaneMesh()
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	glm::vec3 vertex;
	glm::vec3 normal;

	// AJOUTER CODE ICI
	// REMPLACER LE CODE DES 2 TRIANGLES POUR FORMER UN PLAN DONT
	// LA RESOLUTION DEPEND DE LA VARIABLE GLOBALE "objectResolution"
	// ==> TOUCHES 'A' ET 'a' DU CLAVIER.


	vertex.y = -100.f;
	float xSize = 200.f, xOrigin = -100.f, xIncrem = xSize / objectResolution;
	float zSize = 200.f, zOrigin = -100.f, zIncrem = zSize / objectResolution;

	normal.x = 0.f; normal.y = 1.f; normal.z = 0.f;

	for (int i = 0; i < objectResolution; i++) {
		for (int j = 0; j < objectResolution; j++) {

			float x = xOrigin + i * xIncrem;
			float z = zOrigin + j * zIncrem;

			// First triangle
			vertex.x = x + xIncrem;
			vertex.z = z + zIncrem;
			vertices.push_back(vertex);
			normals.push_back(normal);

			vertex.x = x + xIncrem;
			vertex.z = z;
			vertices.push_back(vertex);
			normals.push_back(normal);

			vertex.x = x;
			vertex.z = z + zIncrem;
			vertices.push_back(vertex);
			normals.push_back(normal);

			// Second triangle
			vertex.x = x + xIncrem;
			vertex.z = z;
			vertices.push_back(vertex);
			normals.push_back(normal);

			vertex.x = x;
			vertex.z = z;
			vertices.push_back(vertex);
			normals.push_back(normal);

			vertex.x = x;
			vertex.z = z + zIncrem;
			vertices.push_back(vertex);
			normals.push_back(normal);
		}
	}

	glBindVertexArray(vaoPlaneID);

	glBindBuffer(GL_ARRAY_BUFFER, vboPlaneID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	int in_PositionLocation = glGetAttribLocation(shader->id(), "in_Position");
	glEnableVertexAttribArray(in_PositionLocation);
	glVertexAttribPointer(in_PositionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, nboPlaneID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	int in_NormalLocation = glGetAttribLocation(shader->id(), "in_Normal");
	glEnableVertexAttribArray(in_NormalLocation);
	glVertexAttribPointer(in_NormalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
}


/*****************************************************
NAME: drawPlane

DESCRIPTION: draw the plane

*****************************************************/
void drawPlane()
{
	shader->bind(); // activate shader

	glm::mat4 planeMatrix = glm::mat4(1.f);

	// Get the locations of uniform shader variables
	int projectionMatrixLocation = glGetUniformLocation(shader->id(), "projectionMatrix");
	int viewMatrixLocation = glGetUniformLocation(shader->id(), "viewMatrix");
	int modelMatrixLocation = glGetUniformLocation(shader->id(), "modelMatrix");

	int materialKaLocation = glGetUniformLocation(shader->id(), "materialKa");
	int materialKdLocation = glGetUniformLocation(shader->id(), "materialKd");
	int materialKsLocation = glGetUniformLocation(shader->id(), "materialKs");
	int materialShininessLocation = glGetUniformLocation(shader->id(), "materialShininess");

	// Copy data to the GPU
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &planeMatrix[0][0]);

	glUniform3f(materialKaLocation, materials[0].ambient[0], materials[0].ambient[1], materials[0].ambient[2]);
	glUniform3f(materialKdLocation, materials[0].diffuse[0], materials[0].diffuse[1], materials[0].diffuse[2]);
	glUniform3f(materialKsLocation, materials[0].specular[0], materials[0].specular[1], materials[0].specular[2]);
	glUniform1f(materialShininessLocation, materials[0].shininess);

	// Draw the object
	glBindVertexArray(vaoPlaneID);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboPlaneID);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, nboPlaneID);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, objectResolution * objectResolution * 6);

	glBindVertexArray(0);

	shader->unbind(); // deactivate shader
}


/*****************************************************
NAME: updateRevolutionObject

DESCRIPTION: create the vertices and the normals of the revolutionObject and
copy it on the GPU (1 normal per vertex).  The number of slices (or meridan) on the
revolution object is defined by the global variable "objectResolution"

*****************************************************/
void updateRevolutionObjectMesh()
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;

	std::vector<glm::vec3> temp;

	glm::vec3 vertex;
	glm::vec3 normal;

	normal.x = 0.f; normal.y = 1.f; normal.z = 0.f;

	float tetha = deg2rad(360) / objectResolution;

	for (int n = 0; n < NB_PTS_ON_SILHOUETTE - 1; n++) {

		float currentAngle = 0.0f;

		std::vector<glm::vec3> temp;

		for (int k = 0; k < objectResolution; k++) {

			vertex.x = (silhouettePointArray[(n)].x)*cos(currentAngle);
			vertex.y = silhouettePointArray[(n)].y;
			vertex.z = -(silhouettePointArray[(n)].x)*sin(currentAngle);

			normals.push_back(getNormalForVertex(n, currentAngle, tetha));
			vertices.push_back(vertex);

			vertex.x = (silhouettePointArray[(n)].x)*cos(currentAngle + tetha);
			vertex.y = silhouettePointArray[(n)].y;
			vertex.z = -(silhouettePointArray[(n)].x)*sin(currentAngle + tetha);

			normals.push_back(getNormalForVertex(n, currentAngle + tetha, tetha));
			vertices.push_back(vertex);

			vertex.x = (silhouettePointArray[(n + 1)].x)*cos(currentAngle);
			vertex.y = silhouettePointArray[(n + 1)].y;
			vertex.z = -(silhouettePointArray[(n + 1)].x)*sin(currentAngle);

			normals.push_back(getNormalForVertex(n + 1, currentAngle, tetha));
			vertices.push_back(vertex);

			vertex.x = (silhouettePointArray[(n)].x)*cos(currentAngle + tetha);
			vertex.y = silhouettePointArray[(n)].y;
			vertex.z = -(silhouettePointArray[(n)].x)*sin(currentAngle + tetha);

			normals.push_back(getNormalForVertex(n, currentAngle + tetha, tetha));
			vertices.push_back(vertex);

			vertex.x = (silhouettePointArray[(n + 1)].x)*cos(currentAngle);
			vertex.y = silhouettePointArray[(n + 1)].y;
			vertex.z = -(silhouettePointArray[(n + 1)].x)*sin(currentAngle);

			normals.push_back(getNormalForVertex(n + 1, currentAngle + tetha, tetha));
			vertices.push_back(vertex);

			vertex.x = (silhouettePointArray[(n + 1)].x)*cos(currentAngle + tetha);
			vertex.y = silhouettePointArray[(n + 1)].y;
			vertex.z = -(silhouettePointArray[(n + 1)].x)*sin(currentAngle + tetha);

			normals.push_back(getNormalForVertex(n + 1, currentAngle + tetha, tetha));
			vertices.push_back(vertex);


			/*vertices.push_back(temp[0]);
			normals.push_back(glm::cross((temp[1] - temp[0]), (temp[2] - temp[0])));

			vertices.push_back(temp[1]);
			normals.push_back(glm::cross((temp[0] - temp[1]), (temp[2] - temp[0])));

			vertices.push_back(temp[2]);
			normals.push_back(glm::cross((temp[0] - temp[2]), (temp[1] - temp[2])));

			vertices.push_back(temp[3]);
			normals.push_back(glm::cross((temp[3] - temp[1]), (temp[2] - temp[1])));

			vertices.push_back(temp[4]);
			normals.push_back(glm::cross((temp[3] - temp[2]), (temp[1] - temp[2])));

			vertices.push_back(temp[5]);
			normals.push_back(glm::cross((temp[2] - temp[3]), (temp[1] - temp[3])));*/

			currentAngle += tetha;
			/*temp.clear();*/
		}
	}

	glBindVertexArray(vaoRevolutionID);

	glBindBuffer(GL_ARRAY_BUFFER, vboRevolutionID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	int in_PositionLocation = glGetAttribLocation(shader->id(), "in_Position");
	glEnableVertexAttribArray(in_PositionLocation);
	glVertexAttribPointer(in_PositionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, nboRevolutionID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	int in_NormalLocation = glGetAttribLocation(shader->id(), "in_Normal");
	glEnableVertexAttribArray(in_NormalLocation);
	glVertexAttribPointer(in_NormalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

}

/*****************************************************
NAME: drawRevolutionObject

DESCRIPTION: draw the revolution object with material materials[1]

*****************************************************/
void drawRevolutionObject()
{
	shader->bind(); // activate shader

	glm::mat4 revolutionObjectMatrix = glm::mat4(1.f);

	// Get the locations of uniform shader variables
	int projectionMatrixLocation = glGetUniformLocation(shader->id(), "projectionMatrix");
	int viewMatrixLocation = glGetUniformLocation(shader->id(), "viewMatrix");
	int modelMatrixLocation = glGetUniformLocation(shader->id(), "modelMatrix");

	int materialKaLocation = glGetUniformLocation(shader->id(), "materialKa");
	int materialKdLocation = glGetUniformLocation(shader->id(), "materialKd");
	int materialKsLocation = glGetUniformLocation(shader->id(), "materialKs");
	int materialShininessLocation = glGetUniformLocation(shader->id(), "materialShininess");

	// Copy data to the GPU
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &revolutionObjectMatrix[0][0]);

	glUniform3f(materialKaLocation, materials[1].ambient[0], materials[1].ambient[1], materials[1].ambient[2]);
	glUniform3f(materialKdLocation, materials[1].diffuse[0], materials[1].diffuse[1], materials[1].diffuse[2]);
	glUniform3f(materialKsLocation, materials[1].specular[0], materials[1].specular[1], materials[1].specular[2]);
	glUniform1f(materialShininessLocation, materials[1].shininess);

	// Draw the object
	glBindVertexArray(vaoRevolutionID);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboRevolutionID);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, nboRevolutionID);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6 * objectResolution*(NB_PTS_ON_SILHOUETTE - 1));

	glBindVertexArray(0);

	shader->unbind(); // deactivate shader
}


/*****************************************************
NAME: drawLight

DESCRIPTION: draw a light specified by 'lightindex'.  Lightindex is 0 or 1.
The light is NOT shaded and its RGB color is the one stored in
"lights[lightindex].diffuse"

*****************************************************/
void drawLight(int lightindex)
{
	shader->bind(); // activate shader

	glm::mat4 lightModelMatrix = glm::translate(glm::mat4(1.f), glm::vec3(lights[lightindex].position[0], lights[lightindex].position[1], lights[lightindex].position[2]));

	// Get the locations of uniform variables
	int projectionMatrixLocation = glGetUniformLocation(shader->id(), "projectionMatrix");
	int viewMatrixLocation = glGetUniformLocation(shader->id(), "viewMatrix");
	int modelMatrixLocation = glGetUniformLocation(shader->id(), "modelMatrix");
	int lightPosLocation = glGetUniformLocation(shader->id(), "lightPosition");
	int materialKdLocation = glGetUniformLocation(shader->id(), "materialKd");

	// Copy data to the GPU
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &lightModelMatrix[0][0]);
	glUniform4f(lightPosLocation, 0.f, 0.f, 0.f, 0.f);
	glUniform3f(materialKdLocation, lights[lightindex].diffuse[0], lights[lightindex].diffuse[1], lights[lightindex].diffuse[2]);

	// Draw object
	glBindVertexArray(vaoLightID);

	glDrawArrays(GL_TRIANGLES, 0, 10 * 10 * 6);

	glBindVertexArray(0);

	shader->unbind(); // deactivate shader
}



/*****************************************************
NAME: getSphereVertices

DESCRIPTION: get the vertices of a sphere made of
 triangles and centered at the origin.

rad: radius of the sphere
meridian: number of slices
latitude: number of stacks.

*****************************************************/
std::vector<glm::vec3> getSphereVertices(float rad, int meridian, int latitude)
{
	// AJOUTER CODE ICI
	// EFFACER LE CONTENU DE CETTE FONCTION ET REMPLACEZ LE PAR DES VERTEX DEVANT FORMER UNE SPHERE DE RAYON "rad"

	// Silhouette parameters : the silhouette contains 17 2D points 
	//#define		NB_PTS_ON_SILHOUETTE 17
	//	glm::vec2	silhouettePointArray[NB_PTS_ON_SILHOUETTE];
	//	int 		objectResolution = 7;	/* the number of times the silhouette is rotated. */
	//
	// meridian = object resolution = nb of time its rotated
	// latitude = nb of stack = nb point on silhouette
	//rad = distance

	// AJOUTER CODE ICI
	// EFFACER LE CONTENU DE CETTE FONCTION ET REMPLACEZ LE PAR DES VERTEX DEVANT FORMER UNE SPHERE DE RAYON "rad"

	std::vector<glm::vec2> sphereSil(latitude);
	std::vector<glm::vec3> vertices;
	glm::vec3 vertex;
	float theta = deg2rad(360) / latitude;
	float currentAngle = 0.0f;

	for (int i = 0; i < latitude; i++)
	{
		sphereSil[i].x = rad * cos(currentAngle + i*theta);
		sphereSil[i].y = rad * sin(currentAngle + i*theta);
	}
	for (int i = 0; i < meridian; i++) {
		for (int j = 0; j < latitude - 1; j++) {
			vertex.x = sphereSil[j].x * cos(currentAngle);
			vertex.y = sphereSil[j].y;
			vertex.z = -(sphereSil[j].x * sin(currentAngle));

			vertices.push_back(vertex);

			vertex.x = sphereSil[j].x * cos(currentAngle + theta);
			vertex.y = sphereSil[j].y;
			vertex.z = -(sphereSil[j].x * sin(currentAngle + theta));

			vertices.push_back(vertex);

			vertex.x = sphereSil[j + 1].x * cos(currentAngle);
			vertex.y = sphereSil[j + 1].y;
			vertex.z = -(sphereSil[j + 1].x * sin(currentAngle));

			vertices.push_back(vertex);

			vertex.x = sphereSil[j].x * cos(currentAngle + theta);
			vertex.y = sphereSil[j].y;
			vertex.z = -(sphereSil[j].x * sin(currentAngle + theta));

			vertices.push_back(vertex);

			vertex.x = sphereSil[j + 1].x * cos(currentAngle);
			vertex.y = sphereSil[j + 1].y;
			vertex.z = -(sphereSil[j + 1].x * sin(currentAngle));

			vertices.push_back(vertex);

			vertex.x = sphereSil[j + 1].x * cos(currentAngle + theta);
			vertex.y = sphereSil[j + 1].y;
			vertex.z = -(sphereSil[j + 1].x * sin(currentAngle + theta));

			vertices.push_back(vertex);
		}
		currentAngle += theta;
		glBindVertexArray(vaoLightID);


		glBindBuffer(GL_ARRAY_BUFFER, vboLightID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		int in_PositionLocation = glGetAttribLocation(shader->id(), "in_Position");
		glEnableVertexAttribArray(in_PositionLocation);
		glVertexAttribPointer(in_PositionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	}

	return vertices;
}


/*****************************************************
NAME: display

DESCRIPTION: display the main window and all its object

*****************************************************/
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setCameraMatrices();
	copyLightParametersOnGPU();

	drawPlane();
	if (displayRevolutionObject) {
		drawRevolutionObject();
	}
	if (displayNormals && displayRevolutionObject) {
		drawNormals();
	}
	drawLight(0);  // white light
	drawLight(1);  // blue light

	glutSwapBuffers();
}

/*****************************************************
NAME: keyboard

DESCRIPTION: callback function for the keyboard

*****************************************************/
void keyboard(unsigned char key, int x, int y)
{
	GLint polyMode[2];
	switch (key)
	{
		// Light manipulation
	case '1':
	case '2':
		currentLight = &(lights[key - '1']);
		break;

	case '3':
		currentLight = NULL;
		break;

		// Object resolution
	case 'A':
		++objectResolution;
		updatePlaneMesh();
		updateRevolutionObjectMesh();
		updateNormalLines();
		break;
	case 'a':
		--objectResolution;
		if (objectResolution <= 0) {
			objectResolution = 1;
		}
		updatePlaneMesh();
		updateRevolutionObjectMesh();
		updateNormalLines();

		break;

		// Camera parameters
	case 'b':
		gCam.ratio -= 0.05f;
		if (gCam.ratio < 0.05f)
			gCam.ratio = 0.05f;
		printf("%f\n", gCam.ratio);
		break;

	case 'B':
		gCam.ratio += 0.05f;
		if (gCam.ratio > 5.f)
			gCam.ratio = 5.f;
		printf("%f\n", gCam.ratio);
		break;

	case 'c':
		gCam.r -= 10.f;
		if (gCam.r < 1.f)
			gCam.r = 1.f;
		break;

	case 'C':
		gCam.r += 10;
		if (gCam.r > 5000)
			gCam.r = 5000.f;
		break;

	case 'e':
		gCam.fovy -= 1.f;
		if (gCam.fovy < 1) gCam.fovy = 1.f;
		break;

	case 'E':
		gCam.fovy += 1.f;
		if (gCam.fovy > 170) gCam.fovy = 170.f;
		break;


	case 'L': // Enable or disables the light
		isLightEnabled = !isLightEnabled;

	case 'n': // Enable or disable the normals
	case 'N':
		displayNormals = !displayNormals;
		break;

	case 'o': // Enable or disable the revolution object
	case 'O':
		displayRevolutionObject = !displayRevolutionObject;
		break;

	case 27:// To exit
	case 'q':
		exit(0);
		break;

	case 'w':// wireframe
		glGetIntegerv(GL_POLYGON_MODE, polyMode);
		if (polyMode[1] == GL_LINE)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;

		//Change shininess
	case 'z':
		materials[0].shininess -= 1.f;
		if (materials[0].shininess < 1.f)
			materials[0].shininess = 1.f;
		break;
	case 'Z':
		materials[0].shininess += 1.f;
		if (materials[0].shininess > 500.f)
			materials[0].shininess = 500.f;
		break;
	case 'x':
		materials[1].shininess -= 1.f;
		if (materials[1].shininess < 1.f)
			materials[1].shininess = 1.f;
		break;
	case 'X':
		materials[1].shininess += 1.f;
		if (materials[1].shininess > 500.f)
			materials[1].shininess = 500.f;
		break;


	default:
		printf("undefined key [%d]\n", (int)key);
		break;
	}

	glutPostRedisplay();
}

/*****************************************************
NAME: setCameraMatrices

DESCRIPTION: update the view and projection matrices
*****************************************************/
void setCameraMatrices()
{
	float z = gCam.r*cos(deg2rad(gCam.theta))*cos(deg2rad(gCam.phi));
	float x = gCam.r*sin(deg2rad(gCam.theta))*cos(deg2rad(gCam.phi));
	float y = gCam.r*sin(deg2rad(gCam.phi));

	viewMatrix = glm::lookAt(glm::vec3(x, y, z),
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f));

	projectionMatrix = glm::perspective(glm::radians(gCam.fovy), gCam.ratio, gCam.nearCP, gCam.farCP);  // Create our perspective projection matrix

}


/*****************************************************
NAME: mouseButton

DESCRIPTION: mouse button callback
*****************************************************/
void mouseButton(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		lastMouseEvt.button = button;
		lastMouseEvt.x = x;
		lastMouseEvt.y = y;
	}
	else if (state == GLUT_UP) {
		lastMouseEvt.button = -1;
		lastMouseEvt.x = -1;
		lastMouseEvt.y = -1;
	}
}

/*****************************************************
NAME: mouseMove

DESCRIPTION: mouse move callback.  x,y contains the position
of the mouse in pixel units.
*****************************************************/
void mouseMove(int x, int y)
{
	int	dx = x - lastMouseEvt.x;
	int	dy = -y + lastMouseEvt.y;
	lastMouseEvt.x = x;
	lastMouseEvt.y = y;

	switch (lastMouseEvt.button)
	{
	case GLUT_LEFT_BUTTON:
	{
		// Rotation
		gCam.theta -= (float)dx;
		gCam.phi -= (float)dy;
		if (gCam.phi > 89) gCam.phi = 89.f;
		if (gCam.phi < -89) gCam.phi = -89.f;
		break;
	}
	case GLUT_MIDDLE_BUTTON:
	{
		// Move current light
		if (currentLight)
		{
			currentLight->position[0] += (viewMatrix[0][0] * dx + viewMatrix[0][1] * dy + viewMatrix[0][2] * 0 + viewMatrix[0][3] * 1) *gCam.r / 900.f;
			currentLight->position[1] += (viewMatrix[1][0] * dx + viewMatrix[1][1] * dy + viewMatrix[1][2] * 0 + viewMatrix[1][3] * 1) *gCam.r / 900.f;
			currentLight->position[2] += (viewMatrix[2][0] * dx + viewMatrix[2][1] * dy + viewMatrix[2][2] * 0 + viewMatrix[2][3] * 1) *gCam.r / 900.f;
		}
		break;
	}
	case GLUT_RIGHT_BUTTON:
	{
		// Zoom in/out
		gCam.r += (float)(dx - dy);
		if (gCam.r < 1)	gCam.r = 1.f;
		break;
	}
	default:
		return;
	}

	glutPostRedisplay();
}

/*****************************************************
NAME: init

DESCRIPTION: initialize everything
*****************************************************/
void init() {
	// Initialize GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	// Enable depth test
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	// Black background
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);

	/*

	WARNING WARNING WARNING WARNING WARNING

	The shaders should be located in the working directory of the project for them to work.
	By default, Visual studio has it located where the vcxproj file (the project directory) is.
	You can check in the project properties (in the debugging section) what the current
	working directory is.

	Alternatively, you might want to change "shader.vert" and "shader.frag" by
	"C:/Users/jodp1301/imn428/project/shader.vert" and "C:/Users/jodp1301/imn428/project/shader.frag"
	where C:/Users/jodp1301/imn428/project/ is the directory containing the shaders.

	WARNING WARNING WARNING WARNIwNG WARNING

	*/
	shader = new Shader("./shader.vert", "./shader.frag");

	// Objects
	initObjects();
	initMaterials();

	// Camera parameters
	gCam.theta = 0.f;
	gCam.phi = 89.f;
	gCam.r = 500.f;
	gCam.fovy = 45.f;
	gCam.ratio = 1.f;
	gCam.nearCP = 1.f;
	gCam.farCP = 3000.f;

	// Create the vertices of the 2 objects, the normals and the 2 sphere lights and copy it on the GPU
	updatePlaneMesh();
	updateRevolutionObjectMesh();
	updateNormalLines();
}

/*****************************************************
NAME: main

DESCRIPTION: create the window and setup the callback functions
	with the glut library.
*****************************************************/
int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	// Init the openGL window
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(500, 500);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 3);

	glutCreateWindow("IMN428 -- TP3 ");

	init();

	// Callbacks
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutMainLoop();

	return 0;
}