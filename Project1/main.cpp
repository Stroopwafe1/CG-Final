#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "LightSource.h"
#include "Material.h"
#include "glsl.h"
#include "objloader.h"
#include "texture.h"
#include "SceneObject.h"
#include "Colour.h"
#include "Shader.h"
#include "MathsHelper.h"
#include "ObjectFactory.h"

//--------------------------------------------------------------------------------
// Consts
//--------------------------------------------------------------------------------

const int WIDTH = 800, HEIGHT = 600;

const char* vertexshader_name = "vertexshader.vert";

unsigned const int DELTA_TIME = 10;

//--------------------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------------------

std::vector<SceneObject*> objects;

// Matrices
glm::mat4 view, projection;

LightSource light;
Material material[2];

glm::vec3 cameraPos = glm::vec3(0.0f, 1.75f, 3.0f); // Position of the camera, at y=1.75
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // What way the camera is facing
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // What axis is 'up'
glm::vec3 lastWalkPos = cameraPos; // The position of the camera when switched to drone mode
glm::vec3 lastWalkFront = cameraFront; // The facing of the camera when switched to drone mode
float yaw, pitch = 0; // Yaw is the direction (in degrees) where you're looking, pitch is up/down (clamped to -89, +89)
float lastWalkYaw, lastWalkPitch; // Yaw and pitch before switched to drone mode
int lastMouseX, lastMouseY = 0; // The last position of the mouse, used for calculating the movement
bool keystates[256]; // All possible keystates of the keyboard within glut, mapped to the int value of the char
bool mouseTrackingToggle = true; // A toggle for first-time focus within the game
bool walkMode = true; // Default walking mode or drone mode
bool animationOn = true; // Default animation on or off
bool isJumping = false, isFalling = false; // Booleans for jumping logic
bool debugMode = true; // Default for debug mode (Text printed on screen)
float eyePos = 1.75f; // Eye position to reset cameraPos to

/*
Cleans up all the heap-allocated variables
*/
void Cleanup() {
	for (int i = 0; i < objects.size(); i++) {
		if (objects.at(i) != nullptr) {
			delete objects.at(i);
			objects.at(i) = nullptr;
		}
	}
	objects.clear();
}

//--------------------------------------------------------------------------------
// Keyboard handling
//--------------------------------------------------------------------------------

/*
Handles the key DOWN event, sets the keystate to true for the pressed key
*/
void keyboardDownHandler(unsigned char key, int x, int y) {
	keystates[key] = true;
	switch (key) {
	case 'v':
		walkMode = !walkMode;
		if (walkMode) {
			cameraPos = lastWalkPos;
			cameraFront = lastWalkFront;
			yaw = lastWalkYaw;
			pitch = lastWalkPitch;
		} else {
			lastWalkPos = cameraPos;
			lastWalkFront = cameraFront;
			lastWalkYaw = yaw;
			lastWalkPitch = pitch;
			cameraPos = glm::vec3(-50, 70, +50);
			pitch = -50;
			yaw = -60;
		}
		break;
	case 27:
		Cleanup();
		glutExit();
		break;
	case ']':
		debugMode = !debugMode;
		break;
	case 'A':
		animationOn = !animationOn;
		break;
	case ' ':
		isJumping = true;
		break;
	}
}

/*
Handles the key UP event, sets the keystate to false for the pressed key
*/
void keyboardUpHandler(unsigned char key, int x, int y) {
	keystates[key] = false;
}

/*
Handles the movement of the camera based on the keystates, allows for smoother movement
*/
void movementHandler() {
	float speedMult = 1;
	if (!walkMode)
		speedMult = 4;
	float cameraSpeed = 0.015f * DELTA_TIME * speedMult;
	float rotationSpeed = 10 * cameraSpeed / speedMult;
	if (keystates['w'])
		cameraPos += cameraSpeed * cameraFront;
	if (keystates['a'])
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keystates['s'])
		cameraPos -= cameraSpeed * cameraFront;
	if (keystates['d'])
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keystates['q']) // Directly down
		cameraPos -= cameraUp * cameraSpeed;
	if (keystates['e']) // Directly up
		cameraPos += cameraUp * cameraSpeed;

	if (keystates['i']) // Look up
		pitch += rotationSpeed;
	if (keystates['j']) { // Look left
		yaw -= rotationSpeed;
		if (yaw <= -360)
			yaw = 0;
	}
	if (keystates['k']) // Look down
		pitch -= rotationSpeed;
	if (keystates['l']) { // Look right
		yaw += rotationSpeed;
		if (yaw >= 360)
			yaw = 0;
	}

	if (walkMode && !isJumping)
		cameraPos.y = eyePos;
}

/*
Handles the passive motion within the window.
Calculates the yaw and pitch based on the positions of the cursor since last time frame
*/
void mouseMotionHandler(int x, int y) {
	if (x == WIDTH / 2 && y == HEIGHT / 2) return;
	if (mouseTrackingToggle) {
		lastMouseX = x;
		lastMouseY = y;
		mouseTrackingToggle = false;
	}
	float xoffset = x - lastMouseX;
	float yoffset = lastMouseY - y;
	lastMouseX = x;
	lastMouseY = y;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw   += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	if (x < 100 || x > WIDTH - 100) {
		lastMouseX = WIDTH / 2;
		lastMouseY = HEIGHT / 2;
		glutWarpPointer(WIDTH / 2, HEIGHT / 2);
	} else if (y < 100 || y > HEIGHT - 100) {
		lastMouseX = WIDTH / 2;
		lastMouseY = HEIGHT / 2;
		glutWarpPointer(WIDTH / 2, HEIGHT / 2);
	}

	if (yaw >= 360 || yaw <= -360)
		yaw = 0;
}

/*
Returns the index of the object in the objects vector by name. Returns -1 if not found
@param name - The name of the object to search for
@returns - The index of the object in the objects vector or -1 if not found
*/
int GetObjectByName(const char* name) {
	for (int i = 0; i < objects.size(); i++) {
		if (objects.at(i)->Name == name)
			return i;
	}
	return -1;
}

//--------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------
/*
Function to render text to the screen.
@param x - The x position
@param y - The y position
@param font - A void pointer to the desired font
@param string - The text to render
@param rgb - The colour of the text
*/
void RenderString(float x, float y, void* font, const char* string, Colour const& rgb) {
	glColor4f(rgb.r, rgb.g, rgb.b, 1.0f);

	// glRasterPos uses coordinates based on a -1 to 1 scale, so to go from absolute positions to this range I use linear interpolation (lerp)
	float X = MathsHelper::lerp(-1, 1, x / WIDTH);
	float Y = MathsHelper::lerp(-1, 1, (y + 12) / HEIGHT) * -1; // *-1 because else it would start at the bottom
	glRasterPos2f(X, Y);

	glutBitmapString(font, (unsigned char*)string);
}

/*
Renders the debug information that helped me debug my code while working
*/
void RenderDebugInformation() {
	Colour colour(1.0f, 1.0f, 0.0f);
	RenderString(0, 0, GLUT_BITMAP_HELVETICA_12, ("Yaw: " + std::to_string(yaw)).c_str(), colour);
	RenderString(0, 14, GLUT_BITMAP_HELVETICA_12, ("Pitch: " + std::to_string(pitch)).c_str(), colour);
	RenderString(0, 28, GLUT_BITMAP_HELVETICA_12, "Camera Pos: ", Colour(0, 1, 0));
	RenderString(14, 42, GLUT_BITMAP_HELVETICA_12, ("Camera Pos X: " + std::to_string(cameraPos.x)).c_str(), colour);
	RenderString(14, 56, GLUT_BITMAP_HELVETICA_12, ("Camera Pos Y: " + std::to_string(cameraPos.y)).c_str(), colour);
	RenderString(14, 70, GLUT_BITMAP_HELVETICA_12, ("Camera Pos Z: " + std::to_string(cameraPos.z)).c_str(), colour);
	RenderString(0, 84, GLUT_BITMAP_HELVETICA_12, "Camera Front: ", Colour(0, 1, 0));
	RenderString(14, 96, GLUT_BITMAP_HELVETICA_12, ("Camera Front X: " + std::to_string(cameraFront.x)).c_str(), colour);
	RenderString(14, 110, GLUT_BITMAP_HELVETICA_12, ("Camera Front Y: " + std::to_string(cameraFront.y)).c_str(), colour);
	RenderString(14, 124, GLUT_BITMAP_HELVETICA_12, ("Camera Front Z: " + std::to_string(cameraFront.z)).c_str(), colour);
	RenderString(0, 138, GLUT_BITMAP_HELVETICA_12, ("Walking Mode: " + std::to_string(walkMode)).c_str(), colour);
	RenderString(0, 152, GLUT_BITMAP_HELVETICA_12, ("Animation: " + std::to_string(animationOn)).c_str(), colour);
	SceneObject* car = objects.at(GetObjectByName("Car"));
	RenderString(0, 166, GLUT_BITMAP_HELVETICA_12, "Car Pos: ", Colour(0, 1, 0));
	RenderString(14, 180, GLUT_BITMAP_HELVETICA_12, ("Car Pos X: " + std::to_string(car->m_Position.x)).c_str(), colour);
	RenderString(14, 194, GLUT_BITMAP_HELVETICA_12, ("Car Pos Y: " + std::to_string(car->m_Position.y)).c_str(), colour);
	RenderString(14, 208, GLUT_BITMAP_HELVETICA_12, ("Car Pos Z: " + std::to_string(car->m_Position.z)).c_str(), colour);
	RenderString(0, 222, GLUT_BITMAP_HELVETICA_12, "Car Rot: ", Colour(0, 1, 0));
	RenderString(14, 236, GLUT_BITMAP_HELVETICA_12, ("Car Rot X: " + std::to_string(car->m_Rotation.x)).c_str(), colour);
	RenderString(14, 250, GLUT_BITMAP_HELVETICA_12, ("Car Rot Y: " + std::to_string(car->m_Rotation.y)).c_str(), colour);
	RenderString(14, 264, GLUT_BITMAP_HELVETICA_12, ("Car Rot Z: " + std::to_string(car->m_Rotation.z)).c_str(), colour);
}

/*
Animates the models with an animation set
*/
void RenderAnimation() {
	for (int i = 0; i < objects.size(); i++) {
		objects.at(i)->Animate();
	}
}

/*
The main render method
*/
void Render() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 direction{};
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	movementHandler();

	if (walkMode && isJumping) {
		if (cameraPos.y <= 2.5 && !isFalling)
			cameraPos.y += 0.05f;
		else {
			isFalling = true;
			cameraPos.y -= 0.05f;
		}
		if (cameraPos.y <= 1.75 && isFalling) {
			isJumping = false;
			isFalling = false;
		}
	}

	for (int i = 0; i < objects.size(); i++) {
		objects.at(i)->Render(&view);
	}
	if (animationOn)
		RenderAnimation();
	if (debugMode)
		RenderDebugInformation();
	else
		RenderString(0, 4, GLUT_BITMAP_HELVETICA_12, "Enter debug mode: ']'", Colour(0, 1, 0));
	glutSwapBuffers();
}

/*
Render method that is called by the timer function
*/
void Render(int n) {
	Render();
	glutTimerFunc(DELTA_TIME, Render, 0);
}

/*
Initializes Glut and Glew
*/
void InitGlutGlew(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("OpenGL assigment Lilith Houtjes");
	glutDisplayFunc(Render);
	glutIgnoreKeyRepeat(GLUT_DEVICE_IGNORE_KEY_REPEAT);
	glutKeyboardFunc(keyboardDownHandler);
	glutKeyboardUpFunc(keyboardUpHandler);
	glutPassiveMotionFunc(mouseMotionHandler);
	glutTimerFunc(DELTA_TIME, Render, 0);

	glewInit();
}

/*
Initialises the shaders for each object depending on their shader type.
The solution only has 2 variants of shading, Shader::SHINY and Shader::MATTE.
If there were more shaders, a switch case would be used.
For modularity, add the filename of the fragment shader in the array below and add a shader type in the Shader enum in Shader.h
*/
void InitShaders() {
	const char* fragment_shaders[] ={
		"fragmentshader_shiny.frag",
		"fragmentshader_matte.frag",
	};
	// One-liner
	// The reason this isn't used is because it depends on the order of the shader types as they appear in the enum
	// objects.at(i)->SetShader(fragment_shaders[(int)objects.at(i)->GetShader()], vertexshader_name);
	for (int i = 0; i < objects.size(); i++) {
		if (objects.at(i)->GetShader() == Shader::SHINY)
			objects.at(i)->SetShader(fragment_shaders[0], vertexshader_name);
		else
			objects.at(i)->SetShader(fragment_shaders[1], vertexshader_name);
	}
}

/*
Initialises the objects into the scene.
There are 2 options for this:
	1. Factory, and creating the object in a fluent manner
	2. Calling the constructor for the object itself
*/
void InitObjects() {
	ObjectFactory* factory = new ObjectFactory();
	SceneObject* building = factory->New()
		->WithName("Talentenplein Building 1")
		->FromObjectModel("Objects/talentenplein_buildingV2.obj")
		->WithTexture("Textures/Yellobrk.bmp")
		->WithShader(Shader::MATTE)
		->WithPosition(glm::vec3(30, 0, 20))
		->WithRotation(glm::radians(270.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* building1 = factory->New()
		->WithName("Talentenplein Building 2")
		->FromObjectModel("Objects/talentenplein_buildingV2.obj")
		->WithTexture("Textures/Yellobrk.bmp")
		->WithShader(Shader::MATTE)
		->WithPosition(glm::vec3(-10, 0, 30))
		->WithRotation(glm::radians(180.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* building2 = factory->New()
		->WithName("Talentenplein Building 3")
		->FromObjectModel("Objects/talentenplein_buildingV2.obj")
		->WithTexture("Textures/Yellobrk.bmp")
		->WithShader(Shader::MATTE)
		->WithPosition(glm::vec3(-60, 0, 15))
		->WithRotation(glm::radians(180.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* building3 = factory->New()
		->WithName("Talentenplein Building 4")
		->FromObjectModel("Objects/talentenplein_buildingV2.obj")
		->WithTexture("Textures/Yellobrk.bmp")
		->WithShader(Shader::MATTE)
		->WithPosition(glm::vec3(-67, 0, -40))
		->WithRotation(glm::radians(90.0f), glm::vec3(0, 1, 0))
		->WithScale(glm::vec3(-1, 1, 1))
		->Build();

	SceneObject* streetlantern = factory->New()
		->WithName("Street Lantern 1")
		->FromObjectModel("Objects/streetlantern.obj")
		->WithTexture("Textures/metal.bmp")
		->WithShader(Shader::SHINY)
		->WithPosition(glm::vec3(13, 0, 75))
		->Build();

	SceneObject* streetlantern1 = factory->New()
		->WithName("Street Lantern 2")
		->FromObjectModel("Objects/streetlantern.obj")
		->WithTexture("Textures/metal.bmp")
		->WithShader(Shader::SHINY)
		->WithPosition(glm::vec3(13, 0, 50))
		->WithRotation(glm::radians(180.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* dimence = factory->New()
		->WithName("Dimence")
		->FromObjectModel("Objects/talentenplein_dimence.obj")
		->WithTexture("Textures/Yellobrk.bmp")
		->WithShader(Shader::MATTE)
		->WithPosition(glm::vec3(0, 0, -45))
		->WithRotation(glm::radians(270.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* basketball = factory->New()
		->WithName("Basketball")
		->FromObjectModel("Objects/talentenplein_basketball.obj")
		->WithTexture("Textures/basketball_texture.bmp")
		->WithShader(Shader::MATTE)
		->WithScale(glm::vec3(1.25, 1.25, 1.25))
		->WithPosition(glm::vec3(-12, 0, -5))
		->WithRotation(glm::radians(90.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* generalWaste = factory->New()
		->WithName("General Wastebin")
		->FromObjectModel("Objects/talentenplein_container.obj")
		->WithTexture("Textures/trashbin_texture.bmp")
		->WithShader(Shader::SHINY)
		->WithPosition(glm::vec3(-26, 0, 0))
		->WithRotation(glm::radians(270.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* paper_recycling = factory->New()
		->WithName("Paper recycling")
		->FromObjectModel("Objects/talentenplein_container.obj")
		->WithTexture("Textures/paper_recycling.bmp")
		->WithShader(Shader::SHINY)
		->WithPosition(glm::vec3(-26, 0, -5))
		->WithRotation(glm::radians(270.0f), glm::vec3(0, 1, 0))
		->Build();

	SceneObject* busstop = new SceneObject("Busstop", "Objects/busstop.obj", "Textures/busstop_texture.bmp", Shader::SHINY);
	SceneObject* tree = new SceneObject("Tree", "Objects/tree.obj", "Textures/treebark.bmp", Shader::MATTE);
	SceneObject* car = new SceneObject("Car", "Objects/cybertruck.obj", "Textures/metal.bmp", Shader::SHINY);
	SceneObject* ground = new SceneObject("Ground", "Objects/ground.obj", "Textures/grass.bmp", Shader::MATTE);

	objects.push_back(building);
	objects.push_back(building1);
	objects.push_back(building2);
	objects.push_back(building3);
	objects.push_back(busstop);
	objects.push_back(streetlantern);
	objects.push_back(streetlantern1);
	objects.push_back(tree);
	objects.push_back(car);
	objects.push_back(ground);
	objects.push_back(dimence);
	objects.push_back(basketball);
	objects.push_back(generalWaste);
	objects.push_back(paper_recycling);
	delete factory;
}

/*
Initialises the animations for objects not made with the factory pattern.
Animations are setup in stages
Each stage has a type of transformation, a speed, the required vec3 for that transformation, an end of the stage, and an optional additional argument (used for rotations)
After all stages are done it will either: loop back to the first (default), not repeat at all, or go through them in reverse order
*/
void InitAnimations() {
	int carIndex = GetObjectByName("Car");
	if (carIndex > 0) {
		Animation* carAnimation = new Animation(AnimationRepeat::REPEAT);
		carAnimation->AddStage(AnimationStage(AnimationType::MOVETO, glm::vec3(0, 0.5f, 17), 150));
		carAnimation->AddStage(AnimationStage(AnimationType::ROTATE, glm::vec3(0, 1, 0), 90, glm::radians(-1.0f)));
		carAnimation->AddStage(AnimationStage(AnimationType::MOVETO, glm::vec3(-30, 0.5f, 17), 100));
		carAnimation->AddStage(AnimationStage(AnimationType::ROTATE, glm::vec3(0, 1, 0), 90, glm::radians(-1.0f)));
		carAnimation->AddStage(AnimationStage(AnimationType::MOVETO, glm::vec3(-30, 0.5f, -27), 150));
		carAnimation->AddStage(AnimationStage(AnimationType::ROTATE, glm::vec3(0, 1, 0), 90, glm::radians(-1.0f)));
		carAnimation->AddStage(AnimationStage(AnimationType::MOVETO, glm::vec3(0, 0.5f, -27), 150));
		carAnimation->AddStage(AnimationStage(AnimationType::ROTATE, glm::vec3(0, 1, 0), 90, glm::radians(-1.0f)));
		objects.at(carIndex)->SetAnimation(carAnimation);
	}
}

/*
Initialises the view and projection matrix
*/
void InitMatrices() {
	view = glm::lookAt(
		glm::vec3(2.0, 2.0, 4.0),  // eye
		glm::vec3(0.0, 0.5, 0.0),  // center
		glm::vec3(0.0, 1.0, 0.0));  // up
	projection = glm::perspective(
		glm::radians(60.0f),
		1.0f * WIDTH / HEIGHT, 0.1f,
		190.0f);
}

/*
Initialises the light source and the materials
*/
void InitLightAndMaterials() {
	light.position = glm::vec3(4.0, 4.0, 4.0);
	material[0].ambient_colour = glm::vec3(0.2, 0.2, 0.1);
	material[0].diffuse_colour = glm::vec3(0.5, 0.5, 0.3);
	material[0].specular = glm::vec3(1);
	material[0].power = 1024;

	material[1].ambient_colour = glm::vec3(0.2, 0.2, 0.1);
	material[1].diffuse_colour = glm::vec3(0.5, 0.5, 0.3);
	material[1].specular = glm::vec3(0.7);
	material[1].power = 4;

	for (int i = 0; i < objects.size(); i++) {
		objects.at(i)->SetMaterial(&material[1]);
		objects.at(i)->SetLight(&light);
	}

	objects.at(0)->SetMaterial(&material[0]);
}

/*
Positions the objects in the scene that were not made through the factory
*/
void PositionObjectsInScene() {
	int busstopIndex = GetObjectByName("Busstop");
	if (busstopIndex > 0) {
		objects.at(busstopIndex)->Translate(glm::vec3(50, 0, 85));
		objects.at(busstopIndex)->Scale(glm::vec3(1.5f));
		objects.at(busstopIndex)->Rotate(glm::radians(180.0f), glm::vec3(0, 1, 0));
	}
	int streetlampIndex = GetObjectByName("Street Lantern");
	if (streetlampIndex > 0) {
		objects.at(streetlampIndex)->Rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		objects.at(streetlampIndex)->Scale(glm::vec3(1.0f, 0.75f, 1.0f));
	}
	int treeIndex = GetObjectByName("Tree");
	if (treeIndex > 0) {
		objects.at(treeIndex)->Translate(glm::vec3(2.5f, 0, 0));
		objects.at(treeIndex)->Scale(glm::vec3(0.75, 0.5, 0.75));
	}
	int carIndex = GetObjectByName("Car");
	if (carIndex > 0) {
		objects.at(carIndex)->Translate(glm::vec3(0, 0.5f, 10));
	}
	int groundIndex = GetObjectByName("Ground");
	if (groundIndex > 0) {
		objects.at(groundIndex)->Translate(glm::vec3(0, -0.01f, 0));
	}
}

/*
Initialises the buffers for each object
*/
void InitBuffers() {
	for (int i = 0; i < objects.size(); i++) {
		objects.at(i)->InitBuffers(&view, &projection);
	}
}

int main(int argc, char** argv) {
	InitGlutGlew(argc, argv);
	InitObjects();
	InitLightAndMaterials();
	InitShaders();
	InitMatrices();
	InitBuffers();
	InitAnimations();
	PositionObjectsInScene();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// Main loop
	glutMainLoop();

	Cleanup();

	return 0;
}