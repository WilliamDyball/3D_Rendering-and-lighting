#include <GL/glew/glew.h>
#include <GL/freeglut.h>
#include <CoreStructures\CoreStructures.h>
#include "CGModel\CGModel.h"
#include "Importers\CGImporters.h"
#include <iostream>
#include "texture_loader.h"
#include "shader_setup.h"
#include <stack>
#include <math.h>


using namespace std;
using namespace CoreStructures;


// variables to track application quit status
bool quit_app = false;

// variables to track mouse status
bool mDown = false;
int mouse_x, mouse_y;


// uniform variables to access in shader
GLuint gDiffuse; // Shader light diffuse
GLuint gAmbient; // Shader light ambient
GLuint gAttenuation; // Shader light attenuation
GLuint gPosition; // Shader light position

GLuint myShaderProgram;


// Models
CGModel *GabumonModel = NULL;
CGModel *GarurumonModel = NULL;
CGModel *MetalGarurumonModel = NULL;

// Textures
GLuint GabumonTexture = NULL;
GLuint GarurumonTexture = NULL;
GLuint MetalGarurumonTexture = NULL;

// Mesh pointers
CGPolyMesh *meshPointer0=NULL;
CGPolyMesh *meshPointer1=NULL;
CGPolyMesh *meshPointer2=NULL;

CGMaterial GabumonMat;
CGMaterial GarurumonMat;
CGMaterial MetalGarurumonMat;

GUPivotCamera *mainCamera = NULL;

//GUFirstPersonCamera *FPSCamera = NULL;

// animation variable to control orientation of the models
float theta = 0.0f;

#pragma region Light_and_material
//
// Example Light
//
// Example light and material
// Define trigonometric constants
const float pi = 3.142f;
const float rad_coeff = pi / 180.0f;

// Define position of light in polar coodinate terms
float			lightTheta = pi / 4.0f;
float			lightRadius = 6.0f;

// Light properties

// Define initial position
GLfloat lightPosition[] = { cos(lightTheta)*lightRadius, sin(lightTheta)*lightRadius, 0.0f, 1.0f };

// Define initial direction (for this demo the light points towards the origin)
GLfloat lightDirection[] = { -lightPosition[0], -lightPosition[1], -lightPosition[2], 0.0 };

GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };

float	ambientScale = 0.2f;
GLfloat lightAmbient[] = { ambientScale, ambientScale, ambientScale, 1.0f }; // no ambient light
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // diffuse light
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // no specular light

GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

GLfloat	ca = 0.5f, caa = 0.5f, la = 0.0, qa = 0.0; // constant, linear and quadratic attenuation

// Material properties
GLfloat	ambientMaterial[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat diffuseMaterial[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat specularMaterial[] = { 0.5f, 0.5, 0.5f, 1.0f };
GLfloat emissiveMaterial[] = { 0.0f, 0.0, 0.0f, 1.0f }; // emission colour
GLfloat materialShininess = 0.75f;
#pragma endregion




// Variables to control demo
bool	showNormals = true;
bool	showLight = true;


void report_version(void) {

	int majorVersion, minorVersion;

	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	
	cout << "OpenGL version " << majorVersion << "." << minorVersion << "\n\n";
}



void init(void) {

	// initialise glew library
	GLenum err = glewInit();
	
	// ensure glew was initialised successfully before proceeding
	if (err==GLEW_OK)
		cout << "GLEW initialised okay\n";
	else
		cout << "GLEW could not be initialised\n";

	report_version();
	myShaderProgram = setupShaders(string("Resources\\Shaders\\diffuse_vertex_shader.txt"), string("Resources\\Shaders\\diffuse_fragment_shader.txt"));

	// get unifom locations in shader
	gPosition = glGetUniformLocation(myShaderProgram, "lposition");
	gAmbient = glGetUniformLocation(myShaderProgram, "lambient");
	gDiffuse = glGetUniformLocation(myShaderProgram, "ldiffuse");
	gAttenuation = glGetUniformLocation(myShaderProgram, "lattenuation");

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_LIGHT0);

	//Define position and direction (so appear at fixed point in scene)
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	//glEnable(GL_LIGHT1);

	//glLightfv(GL_LIGHT1, GL_LIGHT_MODEL_AMBIENT, );

	glColor3f(0.0f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientMaterial);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuseMaterial);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularMaterial);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &materialShininess);


	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);


	//// setup main 3D camera
	mainCamera = new GUPivotCamera(0.0f, 0.0f, 5.0f, 55.0f, 1.0f, 0.1f);

	//FPSCamera = new GUFirstPersonCamera();
	

#pragma region Load_Models_and_Textures

	// load  Gabumon model
	GabumonModel = new CGModel();
	import3DS(L"Resources\\Models\\Gabumon.3ds", GabumonModel);

	//load Gabumon model texture
	GabumonTexture = loadTexture(wstring(L"Resources\\Textures\\Gabumon.png"));
	GabumonModel->setTextureForModel( GabumonTexture);

	// load  Garurumon model
	GarurumonModel = new CGModel();
	import3DS(L"Resources\\Models\\Garurumon.3ds", GarurumonModel);

	//load Garurumon model texture
	GarurumonTexture = loadTexture(wstring(L"Resources\\Textures\\Garurumon.png"));
	GarurumonModel->setTextureForModel(GarurumonTexture);

	// load  MetalGarurumon model
	MetalGarurumonModel = new CGModel();
	import3DS(L"Resources\\Models\\MetalGarurumon.3ds", MetalGarurumonModel);

	//load MetalGarurumon model texture
	MetalGarurumonTexture = loadTexture(wstring(L"Resources\\Textures\\MetalGarurumon.png"));
	MetalGarurumonModel->setTextureForModel(MetalGarurumonTexture);


#pragma endregion 

	
	//
	// Setup Lighting
	//

	glEnable(GL_NORMALIZE); // If we scale objects, ensure normal vectors are re-normalised to length 1.0 to keep lighting calculations correct (see lecture notes)
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Best colour interpolation results

	// Setup GL_LIGHT0
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);		// Setup ambient light
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);		// Setup diffuse light
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);	// Setup specular light

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, ca);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, la);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, qa);

	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0);

	// OpenGL provides a global ambient light component - we don't want this so set to zero
	GLfloat global_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

}


void display(void) {
	glEnable(GL_TEXTURE_2D);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// get camera view and projection transforms
	GUMatrix4 viewMatrix = mainCamera->viewTransform();
	GUMatrix4 projMatrix = mainCamera->projectionTransform();
	
#if 1

	// 3D models import using CGImport use the older fixed function pipeline of OpenGL 2.1
	glUseProgram(0);
	glBindVertexArray(0);


	// set fixed-function projection transform
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((GLfloat*)&projMatrix);
	
	// set fixed-function model-view transform
	glMatrixMode(GL_MODELVIEW);
	GUMatrix4 T = viewMatrix * GUMatrix4::rotationMatrix(0.0f, 0.0f, theta);
	glLoadMatrixf((GLfloat*)&T);

	// Render point that represents the light source
	//if (showLight) {
	//	glPointSize(5.0f);
	//	glColor3ub(255, 255, 255);
	//	glBegin(GL_POINTS);
	//	glVertex3f(lightPosition[0], lightPosition[1], lightPosition[2]);
	//	glEnd();


	//	// Render line that represents direction of light
	//	glEnable(GL_LINE_STIPPLE);
	//	glLineStipple(1, 0xf0f0);

	//	glBegin(GL_LINES);

	//	glVertex3f(lightPosition[0], lightPosition[1], lightPosition[2]);
	//	glVertex3f(lightPosition[0]+lightDirection[0], lightPosition[1]+lightDirection[1], lightPosition[2]+lightDirection[2]);
	//		
	//	glEnd();

	//	glDisable(GL_LINE_STIPPLE);
	//}


	glEnable(GL_LIGHTING);

	// enable texturing (since models are textured)
	glEnable(GL_TEXTURE_2D);

	// draw  model
	if (1)
	{
		glUseProgram(myShaderProgram);//use shader

		glUniform4fv(gDiffuse, 1, (GLfloat*)&lightDiffuse); //Sets matrials for the mat objects
		glUniform4fv(gAmbient, 1, (GLfloat*)&lightAmbient);
		glUniform4fv(gPosition, 1, (GLfloat*)&lightPosition);
		GLfloat	attenuation[] = { ca, la, qa };
		glUniform3fv(gAttenuation, 1, (GLfloat*)&attenuation);
		specularMaterial[0] = 0.1f;
		specularMaterial[1] = 0.1f;
		specularMaterial[2] = 0.1f;
		materialShininess = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularMaterial);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &materialShininess);

		glPushMatrix();
		glTranslatef(-1.0f, 0.0f, 0.0f);
		glScalef(0.5f, 0.5f, 0.5f);
		GabumonModel->renderTexturedModel();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.0f, 0.0f, 0.0f);
		glScalef(1.25f, 1.25f, 1.25f);
		GarurumonModel->renderTexturedModel();
		glPopMatrix();

		specularMaterial[0] = 1.0f;
		specularMaterial[1] = 1.0f;
		specularMaterial[2] = 1.0f;
		materialShininess = 100.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularMaterial);//Sets matrials for the shiny object
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &materialShininess);
		glPushMatrix();
		glTranslatef(1.5f, 0.0f, 0.0f);
		MetalGarurumonModel->renderTexturedModel();
		glPopMatrix();

		glUseProgram(0);//use shader
	}


	// disable texturing
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

#endif
	

	glutSwapBuffers();
}


void mouseButtonDown(int button_id, int state, int x, int y) {

	if (button_id==GLUT_LEFT_BUTTON) {
		
		if (state==GLUT_DOWN) {

			mouse_x = x;
			mouse_y = y;

			mDown = true;

		} else if (state == GLUT_UP) {

			mDown = false;
		}
	}
}

void mouseMove(int x, int y) {

	if (mDown) {

		int dx = x - mouse_x;
		int dy = y - mouse_y;

		// rotate camera around the origin
		if (mainCamera)
			mainCamera->transformCamera((float)-dy, (float)-dx, 0.0f);
		
		mouse_x = x;
		mouse_y = y;
	}
	
}

void mouseWheelUpdate(int wheel_number, int direction, int x, int y) {

	if (mainCamera) {
		
		if (direction<0)
			mainCamera->scaleCameraRadius(1.1f);
		else if (direction>0)
			mainCamera->scaleCameraRadius(0.9f);
	}
}

void keyDown(unsigned char key, int x, int y) {
	
	switch((int)key) {

		// example keyboard input
		case'r':
			theta = 0.0f;
			glutPostRedisplay();
		break; 
	
			// Change orientation of light's position (move around the object on xz plane)
		case 'j':
			break;

		case 'k':
			break;

			// Change linear attenuation parameters
		//case 'q':
		//	ca *= 1.1f;
		//	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, ca);
		//	glutPostRedisplay();
		//	break;

		//case 'a':
		//	ca *= 0.9f;
		//	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, ca);
		//	glutPostRedisplay();
		//	break;

		// Change light display flags
		case 'l':
			showLight = !showLight;
			glutPostRedisplay();
			break;

		case 'z':

			break;
		case 'x':

			break;

		case VK_ESCAPE:	// Escape
			quit_app = true;
			break;
	}
	
}


int main(int argc, char **argv) {
	
	// initialise GLUT and COM
	glutInit(&argc, argv);
	initCOM();
	
	// declare OpenGL context properties
	glutInitContextVersion(3,3);
	
	glutInitContextProfile (GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	
	// setup window
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 0);
	
	glutCreateWindow("Welcome to the Digiverse!");
	
	// register callback functions with GLUT
	glutDisplayFunc(display);
	glutKeyboardFunc(keyDown);
	glutMouseFunc(mouseButtonDown);
	glutMotionFunc(mouseMove);
	glutMouseWheelFunc(mouseWheelUpdate);

	// initialise OpenGL environment, load models etc...
	init();
	

	// main rendering loop
	while (!quit_app) {

		glutMainLoopEvent(); // internal glut processing (events etc)

		// do our own per-frame processing
		theta += 0.00f;
		lightTheta -= pi / 350.0f;

		lightPosition[0] = cos(lightTheta)*lightRadius;
		lightPosition[2] = sin(lightTheta)*lightRadius;

		lightDirection[0] = -lightPosition[0];
		lightDirection[2] = -lightPosition[2];

		// redraw
		glutPostRedisplay();
	}

	// clean-up COM
	shutdownCOM();

	return 0;
}