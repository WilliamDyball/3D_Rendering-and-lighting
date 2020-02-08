
#pragma once

#include <GL/glew/glew.h>
#include <GL/freeglut.h>
#include "shader_setup.h"
#include <CoreStructures\GUVector4.h>
#include <CoreStructures\GUMatrix4.h>


class star {

	// star instance variables
	CoreStructures::GUVector4		pos;
	CoreStructures::GUMatrix4		orientation;


public:
	
	//
	// class method interface
	//
	static void initStar(void);

	//
	// instance method interface
	//

	// constructors
	star();
	star(float x, float y, float z);
	star(float x, float y, float z, float rx, float ry, float rz);

	// drawing methods
	void render(const CoreStructures::GUMatrix4& cameraTransform);

};