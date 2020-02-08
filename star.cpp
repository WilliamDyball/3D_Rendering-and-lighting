
#include "star.h"

using namespace std;
using namespace CoreStructures;


// Packed vertex arrays for the star object

// 1) Position Array - Store vertices as (x,y) pairs
static GLfloat starVertices [] = {
	
	0.0, 0.25f,
	0.1f, 0.1f,
	0.25f, 0.08f,
	0.15f, -0.05f,
	0.25f, -0.25f,
	0.0, -0.125f,
	-0.25f, -0.25f,
	-0.15f, -0.05f,
	-0.25f, 0.08f,
	-0.1f, 0.1f
};

// 2) Colour Array - Store RGB values as unsigned bytes
static GLubyte starColors [] = {
	
	255, 0, 0, 255,
	255, 255, 0, 255,
	0, 255, 0, 255,
	0, 255, 255, 255,
	0, 0, 255, 255,
	128, 45, 200, 255,
	0, 255, 0, 255,
	255, 0, 255, 255,
	255, 128, 255, 255,
	255, 128, 128, 255
};

// 3) Texture coordinate array (store uv coordinates as floating point values)
static float starTextureCoords [] = {

	0.5f, 1.0f,
	0.6f, 0.7f,
	1.0f, 0.7f,
	0.8f, 0.4f,
	0.95f, 0.0f,
	0.0f, 0.3f,
	-0.95f, 0.0f,
	-0.8f, 0.4f,
	-1.0f, 0.7f,
	-0.6f, 0.7f
};

// 4) Index Array - Store indices to star vertices - this determines the order the vertices are to be processed
static GLubyte starVertexIndices [] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};



// star object VBO data - singleton model (not duplicated between star instances)
GLuint starPosVBO, starColourVBO, starTexCoordVBO, starIndicesVBO, starVAO;

// star shader program
GLuint starShaderProgram;

// uniform variables to access in star shader
GLuint locT; // model-view-projection matrix



//
// class method interface
//


// setup star VBO and VAO data
void setupStarVAO(void) {

	glGenVertexArrays(1, &starVAO);
	glBindVertexArray(starVAO);

	// copy star vertex position data to VBO
	glGenBuffers(1, &starPosVBO);
	glBindBuffer(GL_ARRAY_BUFFER, starPosVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starVertices), starVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

	// copy star vertex colour data to VBO
	glGenBuffers(1, &starColourVBO);
	glBindBuffer(GL_ARRAY_BUFFER, starColourVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starColors), starColors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (const GLvoid*)0);
	
	// copy star texture coordinate data to VBO
	glGenBuffers(1, &starTexCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, starTexCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starTextureCoords), starTextureCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
	
	// enable position, colour and texture coordinate buffer inputs
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// setup star vertex index array
	glGenBuffers(1, &starIndicesVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starIndicesVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(starVertexIndices), starVertexIndices, GL_STATIC_DRAW);

	
	glBindVertexArray(0);
}


// initialise star shaders and VAO (singleton) model
void star::initStar(void) {

	// load shader
	starShaderProgram = setupShaders(string("Resources\\Shaders\\basic_vertex_shader.txt"), string("Resources\\Shaders\\basic_fragment_shader.txt"));

	// get unifom locations in shader
	locT = glGetUniformLocation(starShaderProgram, "T");
	
	// create VAO
	setupStarVAO();
}


//
// instance method interface
//

// constructors
star::star() {

	pos = GUVector4();
	orientation = GUMatrix4::identity();
}


star::star(float x, float y, float z) {

	pos = GUVector4(x, y, z, 1.0f);
	orientation = GUMatrix4::identity();
}


star::star(float x, float y, float z, float rx, float ry, float rz) {

	pos = GUVector4(x, y, z, 1.0f);
	orientation = GUMatrix4::rotationMatrix(rx, ry, rz);
}


// drawing methods
void star::render(const CoreStructures::GUMatrix4& cameraTransform) {

	// use 'starShaderProgram' for drawing
	glUseProgram(starShaderProgram);

	// setup model-view-projection transform
	GUMatrix4 T = cameraTransform * GUMatrix4::translationMatrix(pos.x, pos.y, pos.z) * orientation;

	glUniformMatrix4fv(locT, 1, GL_FALSE, (GLfloat*)&T);


	// tell OpenGL to use star VAO
	glBindVertexArray(starVAO);


	// draw the star object
	glDrawElements(GL_LINE_LOOP, 10, GL_UNSIGNED_BYTE, (GLvoid*)0);
}

