#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <iostream>
#include <cmath>

#define DEG2RAD(a) (a * 0.0174532925)
#define RAD2DEG(a) (a*180 / 3.141592653589793238463)

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 60.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.01;
GLdouble zFar = 300;
GLdouble angle = 0;
GLdouble lx = 1;
GLdouble lz = 1;
float playerAngle = 180;
float playerRotation = 0;
float playerZ = 0;
float ballX = 0;
float ballZ = 0;
int collisioncounter = 0;


bool collided = false;
bool flag = true;
int status = 0; //0 game 1 win 2 lose
int timecounter = 50;
int timers = 0;


class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
	Vector operator-(Vector& v) {
		return Vector(x - v.x, y - v.y, z - v.z);
	}

	Vector operator*(float n) {
		return Vector(x * n, y * n, z * n);
	}

	Vector operator/(float n) {
		return Vector(x / n, y / n, z / n);
	}

	Vector unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector cross(Vector v) {
		return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	float dot(Vector v) {
		return (x * v.x) + (y * v.y) + (z * v.z);
	}

	Vector operator+(Vector& v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}

};

	float mag(Vector v) {
		return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}
Vector Eye(20, 20, 20);
Vector At(0, 5, 0);
Vector Up(0, 1, 0);
Vector player(0, 0, 10);
Vector ball(0, 0, 0);
Vector ballray(0, 0, 0);
Vector trophy(-30, 11, -50);


void moveX(float d) {
	Vector right = Up.cross(At - Eye).unit();
	Eye = Eye + (right * d);
	At = At + (right * d);

}

void moveY(float d) {
	Eye = Eye + Up.unit() * d;
	At = At + Up.unit() * d;

}

void moveZ(float d) {
	Vector depth = (At - Eye).unit();
	Eye = Eye + (depth * d);
	At = At + (depth * d);

}

void rotateX(float a) {
	Vector depth = (At - Eye).unit();
	Vector right = Up.cross(depth).unit();
	depth = depth * cos(DEG2RAD(a)) + Up * sin(DEG2RAD(a));
	Up = depth.cross(right);
	At = Eye + depth;
}

void rotateY(float a) {
	Vector depth = (At - Eye).unit();
	Vector right = Up.cross(depth).unit();
	depth = depth * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
	right = depth.cross(Up);
	At = Eye + depth;
}
void rotateZ(float a) {
	Vector depth = (At - Eye).unit();
	Vector right = Up.cross(depth).unit();	
	Up = Up * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
}



int cameraZoom = 0;

// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_goal;
Model_3DS model_player;
Model_3DS model_ball;

// Textures
GLTexture tex_ground;
GLTexture tex_ball;
GLTexture tex_ghaleez;
GLTexture tex_sherif;
GLTexture tex_palestine;
GLTexture tex_yemen;
GLTexture tex_qatar;

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(50, 0, 50);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 0, 50);
	glTexCoord2f(1, 1);
	glVertex3f(-50, 0, -50);
	glTexCoord2f(1,0);
	glVertex3f(50, 0, -50);
	glEnd();
	glPopMatrix();


	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glColor3ub(95,	45,	34);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.	// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(75, -0.1, 75);
	glVertex3f(-75, -0.1, 75);
	glVertex3f(-75, -0.1, -75);
	glVertex3f(75, -0.1, -75);
	glEnd();
	glPopMatrix();

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.



}

void drawTrophy(int x , int y , int z) {

	glColor3ub(212, 175, 55);

	glPushMatrix();
	glTranslatef(x, y, z);
	glutSolidCube(2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, y+2, z);
	glRotatef(90, 1, 0, 0);
	glutSolidCone(1, 1, 10, 10);
	glPopMatrix();

	
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	GLUquadricObj* qobj2;
	qobj2 = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, tex_ball.texture[0]);
	gluQuadricTexture(qobj2, true);
	gluQuadricNormals(qobj2, GL_SMOOTH);
	glTranslatef(x, y+2.5, z);
	gluSphere(qobj2, 1, 50, 50);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	


}


void drawAd() {


	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(-65, 20, -70);
	glRotatef(90, 1, 0, 0);
	gluCylinder(qobj, 1, 1, 20, 10, 10);
	glPopMatrix();


	glPushMatrix();
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();
	gluQuadricNormals(qobj1, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(-65, 35, -70);
	gluSphere(qobj1, 1, 10, 10);
	glPopMatrix();


	glDisable(GL_LIGHTING);	// Disable lighting 

	//glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit


	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing


	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, tex_ghaleez.texture[0]);	// Bind the ground texture
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-75, 20, -70);
	glTexCoord2f(0, 1);
	glVertex3f(-75, 35, -70);
	glTexCoord2f(1, 1);
	glVertex3f(-55, 35, -70);
	glTexCoord2f(1, 0);
	glVertex3f(-55, 20, -70);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
	glDisable(GL_TEXTURE_2D);


	glPushMatrix();
	GLUquadricObj* qobj2;
	qobj2 = gluNewQuadric();
	gluQuadricNormals(qobj2, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(65, 20, 70);
	glRotatef(90, 1, 0, 0);
	gluCylinder(qobj2, 1, 1, 20, 10, 10);
	glPopMatrix();


	glPushMatrix();
	GLUquadricObj* qobj3;
	qobj3 = gluNewQuadric();
	gluQuadricNormals(qobj3, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(65, 35, 70);
	gluSphere(qobj3, 1, 10, 10);
	glPopMatrix();


	glDisable(GL_LIGHTING);	// Disable lighting 

	//glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit


	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing


	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, tex_ghaleez.texture[0]);	// Bind the ground texture
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(55, 20, 70);
	glTexCoord2f(0, 1);
	glVertex3f(55, 35, 70);
	glTexCoord2f(1, 1);
	glVertex3f(75, 35, 70);
	glTexCoord2f(1, 0);
	glVertex3f(75, 20, 70);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
	glDisable(GL_TEXTURE_2D);


	glPushMatrix();
	GLUquadricObj* qobj4;
	qobj4 = gluNewQuadric();
	gluQuadricNormals(qobj4, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(65, 20, -70);
	glRotatef(90, 1, 0, 0);
	gluCylinder(qobj4, 1, 1, 20, 10, 10);
	glPopMatrix();


	glPushMatrix();
	GLUquadricObj* qobj5;
	qobj5= gluNewQuadric();
	gluQuadricNormals(qobj5, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(65, 35, -70);
	gluSphere(qobj5, 1, 10, 10);
	glPopMatrix();


	glDisable(GL_LIGHTING);	// Disable lighting 

	//glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit


	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing


	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, tex_ghaleez.texture[0]);	// Bind the ground texture
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(55, 20, -70);
	glTexCoord2f(0, 1);
	glVertex3f(55, 35, -70);
	glTexCoord2f(1, 1);
	glVertex3f(75, 35, -70);
	glTexCoord2f(1, 0);
	glVertex3f(75, 20, -70);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
	glDisable(GL_TEXTURE_2D);





}


void drawScoreboard() {

	glPushMatrix();
	GLUquadricObj* qobj4;
	qobj4 = gluNewQuadric();
	gluQuadricNormals(qobj4, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(0, 20, -70);
	glRotatef(90, 1, 0, 0);
	gluCylinder(qobj4, 1, 1, 20, 10, 10);
	glPopMatrix();


	glPushMatrix();
	GLUquadricObj* qobj5;
	qobj5 = gluNewQuadric();
	gluQuadricNormals(qobj5, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(0, 45, -70);
	gluSphere(qobj5, 1, 10, 10);
	glPopMatrix();


	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0, 0, 0);	// Dim the ground texture a bit



	glPushMatrix();
	//glBindTexture(GL_TEXTURE_2D, tex_ghaleez.texture[0]);	// Bind the ground texture
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	//glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 20, -70);
	//glTexCoord2f(0, 1);
	glVertex3f(-20, 45, -70);
	//glTexCoord2f(1, 1);
	glVertex3f(20, 45, -70);
	//(1, 0);
	glVertex3f(20, 20, -70);
	glEnd();
	glPopMatrix();


	glPushMatrix();
	glColor3f(1, 0, 0);
	glBegin(GL_QUADS);
	glVertex3f(-15, 40, -69.9);
	glVertex3f(-10, 40, -69.9);
	glVertex3f(-10, 25, -69.9);
	glVertex3f(-15, 25, -69.5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 0, 0);
	glBegin(GL_QUADS);
	glVertex3f(15, 40, -69.5);
	glVertex3f(10, 40, -69.5);
	glVertex3f(10, 25, -69.5);
	glVertex3f(15, 25, -69.5);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(0, 30, -70);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(0, 35, -70);
	glutSolidCube(1);
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}





void print(int x, int y, char* string)
{
	int len, i;

	//set the position of the text in the window using the x and y coordinates
	glRasterPos2f(x, y);

	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void timershow() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1280, 0, 720);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glColor3f(1, 0, 0);
	char* p0s[20];
	sprintf((char*)p0s, "Time Left: %d" , timecounter);
	print(0, 700, (char*)p0s);
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


}

void endscreen() {
	if (status == 1) {
		
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, 1280, 0, 720);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			glDisable(GL_LIGHTING);
			glColor3f(1, 0, 0);
			char* p0s[20];
			sprintf((char*)p0s, "You Win !! press space");
			print(640, 360, (char*)p0s);
			glEnable(GL_LIGHTING);
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
	}
	else if (status == 2) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, 1280, 0, 720);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_LIGHTING);
		glColor3f(1, 0, 0);
		char* p0s[20];
		sprintf((char*)p0s, "you lose he sad press space");
		print(640, 360, (char*)p0s);
		glEnable(GL_LIGHTING);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}




void drawBarrier() {

	//right side

	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit


	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing


	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);	// Bind the ground texture
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(50, 0, -50);
	glTexCoord2f(0, 1);
	glVertex3f(50, 10, -50);
	glTexCoord2f(1, 1);
	glVertex3f(50, 10, -25);
	glTexCoord2f(1, 0);
	glVertex3f(50, 0, -25);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(50, 0, -25);
	glTexCoord2f(0, 1);
	glVertex3f(50, 10, -25);
	glTexCoord2f(1, 1);
	glVertex3f(50, 10, 0);
	glTexCoord2f(1, 0);
	glVertex3f(50, 0, 0);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(50, 0, 0);
	glTexCoord2f(0, 1);
	glVertex3f(50, 10, 0);
	glTexCoord2f(1, 1);
	glVertex3f(50, 10, 25);
	glTexCoord2f(1, 0);
	glVertex3f(50, 0, 25);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(50, 0, 25);
	glTexCoord2f(0, 1);
	glVertex3f(50, 10, 25);
	glTexCoord2f(1, 1);
	glVertex3f(50, 10, 50);
	glTexCoord2f(1, 0);
	glVertex3f(50, 0, 50);
	glEnd();
	glPopMatrix();

	// ----------------------------------------------------------- front

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-50, 0, -50);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 10, -50);
	glTexCoord2f(1, 1);
	glVertex3f(-25, 10, -50);
	glTexCoord2f(1, 0);
	glVertex3f(-25, 0, -50);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-25, 0, -50);
	glTexCoord2f(0, 1);
	glVertex3f(-25, 10, -50);
	glTexCoord2f(1, 1);
	glVertex3f(0, 10, -50);
	glTexCoord2f(1, 0);
	glVertex3f(0, 0, -50);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0, 0, -50);
	glTexCoord2f(0, 1);
	glVertex3f(0, 10, -50);
	glTexCoord2f(1, 1);
	glVertex3f(25, 10, -50);
	glTexCoord2f(1, 0);
	glVertex3f(25, 0, -50);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(25, 0, -50);
	glTexCoord2f(0, 1);
	glVertex3f(25, 10, -50);
	glTexCoord2f(1, 1);
	glVertex3f(50, 10, -50);
	glTexCoord2f(1, 0);
	glVertex3f(50, 0, -50);
	glEnd();
	glPopMatrix();

	//--------------------------------------------------------------------- left

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-50, 0, -50);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 10, -50);
	glTexCoord2f(1, 1);
	glVertex3f(-50, 10, -25);
	glTexCoord2f(1, 0);
	glVertex3f(-50, 0, -25);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-50, 0, -25);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 10, -25);
	glTexCoord2f(1, 1);
	glVertex3f(-50, 10, 0);
	glTexCoord2f(1, 0);
	glVertex3f(-50, 0, 0);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-50, 0, 0);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 10, 0);
	glTexCoord2f(1, 1);
	glVertex3f(-50, 10, 25);
	glTexCoord2f(1, 0);
	glVertex3f(-50, 0, 25);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-50, 0, 25);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 10, 25);
	glTexCoord2f(1, 1);
	glVertex3f(-50, 10, 50);
	glTexCoord2f(1, 0);
	glVertex3f(-50, 0, 50);
	glEnd();
	glPopMatrix();


	// -------------------------------------------------------- back

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-50, 0, 50);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 10, 50);
	glTexCoord2f(1, 1);
	glVertex3f(-25, 10, 50);
	glTexCoord2f(1, 0);
	glVertex3f(-25, 0, 50);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-25, 0, 50);
	glTexCoord2f(0, 1);
	glVertex3f(-25, 10, 50);
	glTexCoord2f(1, 1);
	glVertex3f(0, 10, 50);
	glTexCoord2f(1, 0);
	glVertex3f(0, 0, 50);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0, 0, 50);
	glTexCoord2f(0, 1);
	glVertex3f(0, 10, 50);
	glTexCoord2f(1, 1);
	glVertex3f(25, 10, 50);
	glTexCoord2f(1, 0);
	glVertex3f(25, 0, 50);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, tex_sherif.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(25, 0, 50);
	glTexCoord2f(0, 1);
	glVertex3f(25, 10, 50);
	glTexCoord2f(1, 1);
	glVertex3f(50, 10, 50);
	glTexCoord2f(1, 0);
	glVertex3f(50, 0, 50);
	glEnd();
	glPopMatrix();


	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
	glDisable(GL_TEXTURE_2D);







}


void drawSkyBox() {

	//sky box
	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing


	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(0, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 200, 200, 200);
	gluDeleteQuadric(qobj);


	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

}

void drawFlags() {

	

	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GL_FLAT);
	glColor3ub(255, 255, 255);					
	glTranslatef(-50, 20, -50);
	glRotatef(90, 1, 0, 0);
	gluCylinder(qobj, 0.5, 0.5, 10, 10, 10);
	glPopMatrix();
																	//first flag
	glPushMatrix();
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();
	gluQuadricNormals(qobj1, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(-50, 20, -50);
	gluSphere(qobj1, 0.5, 10, 10);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, tex_palestine.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-50, 15, -50);
	glTexCoord2f(0, 1);
	glVertex3f(-50, 20, -50);
	glTexCoord2f(1, 1);
	glVertex3f(-65, 20, -50);
	glTexCoord2f(1, 0);
	glVertex3f(-65, 15, -50);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);


	glPushMatrix();
	GLUquadricObj* qobj2;
	qobj2 = gluNewQuadric();
	gluQuadricNormals(qobj2, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(0, 25, -50);
	glRotatef(90, 1, 0, 0);
	gluCylinder(qobj2, 0.5, 0.5, 10, 10, 10);
	glPopMatrix();
																		//second flag
	glPushMatrix();
	GLUquadricObj* qobj3;
	qobj3 = gluNewQuadric();
	gluQuadricNormals(qobj3, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(0, 25, -50);
	gluSphere(qobj3, 0.5, 10, 10);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, tex_qatar.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0, 20, -50);
	glTexCoord2f(0, 1);
	glVertex3f(0, 25, -50);
	glTexCoord2f(1, 1);
	glVertex3f(-15, 25, -50);
	glTexCoord2f(1, 0);
	glVertex3f(-15, 20, -50);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	GLUquadricObj* qobj4;
	qobj4 = gluNewQuadric();
	gluQuadricNormals(qobj4, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(50, 20, -50);
	glRotatef(90, 1, 0, 0);
	gluCylinder(qobj4, 0.5, 0.5, 10, 10, 10);
	glPopMatrix();
																		//third flag
	glPushMatrix();
	GLUquadricObj* qobj5;
	qobj5 = gluNewQuadric();
	gluQuadricNormals(qobj5, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(50, 20, -50);
	gluSphere(qobj5, 0.5, 10, 10);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, tex_yemen.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(50, 15, -50);
	glTexCoord2f(0, 1);
	glVertex3f(50, 20, -50);
	glTexCoord2f(1, 1);
	glVertex3f(35, 20, -50);
	glTexCoord2f(1, 0);
	glVertex3f(35, 15, -50);
	glEnd();
	glPopMatrix();



	glDisable(GL_TEXTURE_2D);



}

void drawPlayer() {

	Vector ray = (ball - player).unit();
	if (ball.x == player.x && ball.z == player.z) {
		ray.x = 0;
		ray.y = 0;
		ray.z = 0;
	}

	//std::cout << anglex << std::endl;

	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(player.x, 0, player.z);
	glScalef(0.2, 0.2, 0.2);
	glRotatef(playerAngle, 0, 1, 0);
	glRotatef(playerRotation, 1, 0, 0);
	model_player.Draw();
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void drawGoal() {
	//glDisable(GL_LIGHTING);
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(12.5,15, -50);
	glRotatef(90, 1, 0, 0);
	//glTranslatef(-12.5, 1, 25);
	gluCylinder(qobj, 1, 1, 15, 15, 15);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj* qobj1;
	qobj1 = gluNewQuadric();
	//gluQuadricNormals(qobj1, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(-12.5, 15, -50);
	glRotatef(90, 1, 0, 0);
	//glTranslatef(-12.5, 1, 25);
	gluCylinder(qobj, 1, 1, 15, 15, 15);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj* qobj2;
	qobj2 = gluNewQuadric();
	//gluQuadricNormals(qobj2, GL_FLAT);
	glTranslatef(-12.5, 15, -50);
	glRotatef(90, 0, 1, 0);
	gluCylinder(qobj2, 1, 1, 25, 25, 25);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj* qobj3;
	qobj3 = gluNewQuadric();
	gluQuadricNormals(qobj3, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(12.5, 15, -40);
	glRotatef(90, 1, 0, 0);
	//glTranslatef(-12.5, 1, 25);
	gluCylinder(qobj3, 1, 1, 15, 15, 15);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj* qobj4;
	qobj4 = gluNewQuadric();
	//gluQuadricNormals(qobj1, GL_FLAT);
	glColor3ub(255, 255, 255);
	glTranslatef(-12.5, 15, -40);
	glRotatef(90, 1, 0, 0);
	//glTranslatef(-12.5, 1, 25);
	gluCylinder(qobj4, 1, 1, 15, 15, 15);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj* qobj5;
	qobj5 = gluNewQuadric();
	//gluQuadricNormals(qobj2, GL_FLAT);
	glTranslatef(-12.5, 15, -40);
	glRotatef(90, 0, 1, 0);
	gluCylinder(qobj5, 1, 1, 25, 25, 25);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj* qobj6;
	qobj6 = gluNewQuadric();
	//gluQuadricNormals(qobj2, GL_FLAT);
	glTranslatef(-12.5, 15, -50);
	glRotatef(90, 0, 0, 1);
	gluCylinder(qobj6, 1, 1, 10, 10, 10);
	glPopMatrix();


	glPushMatrix();
	GLUquadricObj* qobj7;
	qobj7 = gluNewQuadric();
	//gluQuadricNormals(qobj2, GL_FLAT);
	glTranslatef(12.5, 15, -50);
	glRotatef(90, 0, 0, 1);
	gluCylinder(qobj7, 1, 1, 10, 10, 10);
	glPopMatrix();


}

void drawBall() {
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	GLUquadricObj* qobj2;
	qobj2 = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, tex_ball.texture[0]);
	gluQuadricTexture(qobj2, true);
	gluQuadricNormals(qobj2, GL_SMOOTH);
	glTranslatef(ball.x,1, ball.z);
	gluSphere(qobj2, 1, 50, 50);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void drawPodium() {
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(0, 0, 0);
	glutSolidCube(5);
	glPopMatrix();
}

void look() {
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
 }




//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	look();
	


	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 250.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	if (status == 0) {
		timershow();
	}

	// Draw Ground

	// Draw Tree Model
	/*glPushMatrix();
	glTranslatef(10, 0, 0);
	glScalef(0.7, 0.7, 0.7);
	model_tree.Draw();
	glPopMatrix();*/

	// Draw house Model
	drawGoal();

	/*glPushMatrix();
	glScalef(0.01, 0.01, 0.01);
	model_ball.Draw();
	glPopMatrix();*/



	//glDisable(GL_LIGHTING);
	//model_house.Draw();
	drawPlayer();
	
	drawBall();

	drawBarrier();

	drawFlags();
	drawTrophy(trophy.x,trophy.y,trophy.z);

	drawAd();

	drawScoreboard();

	if (status == 1) {
		drawPodium();
		endscreen();
	}
	if (status == 2) {
		endscreen();
	}
	RenderGround();
	//sky box
	drawSkyBox();

	
	



		
	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	float fraction = 1;
	float angle = 1.0;
	if (status == 0) {
		switch (button)
		{
		case 'w':
			moveZ(fraction);
			break;
		case 's':
			moveZ(-fraction);
			break;
		case 'd':
			moveX(-fraction);
			break;
		case 'a':
			moveX(fraction);
			break;
		case 'e':
			moveY(fraction);
			break;
		case 'q':
			moveY(-fraction);
			break;
		case 'i':
			rotateX(angle);
			break;
		case 'k':
			rotateX(-angle);
			break;
		case 'l':
			rotateY(-angle);
			break;
		case 'j':
			rotateY(angle);
			break;
		case 'o':
			rotateZ(-angle);
			break;
		case 'u':
			rotateZ(angle);
			break;

		case '1':
			//top view
			Eye = Vector(0, 105, 0);
			Up = Vector(0, 0, -1);
			At = Vector(0, 0, 0);
			break;
		case '2':
			//front view
			Eye = Vector(0, 25, 95);
			Up = Vector(0, 1, 0);
			At = Vector(0, 25, 0);
			break;
		case '3':
			//side view
			Eye = Vector(95, 25, 0);
			Up = Vector(0, 1, 0);
			At = Vector(0, 25, 0);
			break;


		default:
			break;
		}
	}
	else if (status == 1 || status == 2) {
		if (button == ' ') {
			status = 0;
			playerRotation = 0;
			trophy.x = -30;
			trophy.y = 11;
			trophy.z = -50;
			ball.x = 0;
			ball.y = 0;
			ball.z = 0;
			timers = 0;
			timecounter = 50;
		}
	}

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_house.Load("Models/house/house.3DS");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_goal.Load("models/goal/goal.3ds");
	model_player.Load("models/player/player.3ds");
	model_ball.Load("models/ball/Ball N100714.3ds");

	// Loading texture files
	tex_ground.Load("textures/footballfield.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
	tex_ball.Load("textures/football.bmp");
	tex_sherif.Load("textures/sherif.bmp");
	tex_ghaleez.Load("textures/ghaleez.bmp");
	tex_palestine.Load("textures/palestine.bmp");
	tex_qatar.Load("textures/qatar.bmp");
	tex_yemen.Load("textures/yemen.bmp");

}

void specialKeys(int key ,int x ,int y) {
	float fraction = 0.1f;
	switch (key) {
	case GLUT_KEY_LEFT:
		if (player.x > -49) {
			player.x -= 1;
			playerAngle = -90; //-90
		}
		break;
	case GLUT_KEY_RIGHT:
		if (player.x < 49) {
			player.x += 1;
			playerAngle = 90; //90
		}
		break;
	case GLUT_KEY_UP:
		if (player.z > -49) {
			player.z -= 1;
			playerAngle = 180; // 180
		}
		break;
	case GLUT_KEY_DOWN:
		if (player.z < 49) {
			player.z += 1;
			playerAngle = 0; //0 
		}
		break;
	}
	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters


	glutPostRedisplay();	//Re-draw scene 
}

void idle() {
}

void timer(int value) {

	Vector unit(1, 1, 1);
	if ((player.x < ball.x + 4 && player.x > ball.x - 4) && (player.z < ball.z + 4 && player.z > ball.z - 4) && flag)
	{
		collided = true;
		ballray = (ball - player);
		//std::cout << ray.x << std::endl;
		flag = false;
	}
	if (collided) {
		if (collisioncounter < 300) {
			ball = ball + ballray * 0.1;
			collisioncounter += 1;
			std::cout << ball.x << std::endl;
			std::cout << ball.z << std::endl;

			if (((ball.x + 2 >= 12.5 && ball.x - 2 <= 12.5) || (ball.x + 2 >= -12.5 && ball.x - 2 <= -12.5)) && (ball.z + 3 >= -50 && ball.z - 3 <= -40))
			{
				ballray.x = -ballray.x;
				ballray.z = -ballray.z;
			}
			if (ball.z - 2 <= -42 && ball.z - 2 >= -50 && ball.x - 2 >= -12.5 && ball.x + 2 <= 12.5)
			{
				collisioncounter = 0;
				collided = false;
				flag = true;
				status = 1;
				trophy.x = 0;
				trophy.y = 4;
				trophy.z = 0;
				player.x = 0;
				player.z = 10;
				Eye.x = 20;
				Eye.y = 20;
				Eye.z = 20;
				timecounter = 50;
				timers = 0;
				At.x = 0;
				At.y = 0;
				At.z = 0;
				Up.x = 0;
				Up.y = 1;
				Up.z = 0;

			}

			if ((ball.x + 3 >= 50 || ball.x - 3 <= -50) || (ball.z + 3 >= 50 || ball.z - 3 <= -50))
			{
				ballray.x = -ballray.x;
				ballray.z = -ballray.z;
			}


			if ((player.x < ball.x + 3 && player.x > ball.x - 3) && (player.z < ball.z + 3 && player.z > ball.z - 3)) {
				collisioncounter = 0;
				ballray = (ball - player);
			}
		}
		else {
			collisioncounter = 0;
			collided = false;
			flag = true;
		}
	}
	if (timecounter == 0) {
		status = 2;
		playerRotation = 90;
	}
	else {
		if (timers >= 99 && status == 0) {
			timecounter -= 1;
			timers = 0;
		}
		else if(status == 0) {
			timers += 1;
		}
	}




	glutPostRedisplay();
	glutTimerFunc(10, timer, 0);
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);

	glutSpecialFunc(specialKeys);

	
	glutIdleFunc(idle);

	glutTimerFunc(0,timer,0);

	glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}