#include "ObjectFactory.h"

ObjectFactory::ObjectFactory() {
	object = nullptr;
}

/*
Makes a new object pointer so this factory can be re-used.
NOTE: If the previous object was not assigned and/or not cleared, this is a memory leak!
*/
ObjectFactory* ObjectFactory::New() {
	object = new SceneObject();
	return this;
}

/*
Sets the name of the scene object
@param name - The name to give the object
*/
ObjectFactory* ObjectFactory::WithName(const char* name) {
	object->Name = name;
	return this;
}

/*
Loads the .obj file of the scene object
@param objFilePath - The path where to find the .obj file
*/
ObjectFactory* ObjectFactory::FromObjectModel(const char* objFilePath) {
	object->LoadModel(objFilePath);
	return this;
}

/*
Sets and loads the texture of the scene object
@param bmpFilePath - The path where to find the .bmp file
*/
ObjectFactory* ObjectFactory::WithTexture(const char* bmpFilePath) {
	object->LoadTexture(bmpFilePath);
	return this;
}

/*
Sets the shader type of the scene object
@param shader - The type of shader this object should have
*/
ObjectFactory* ObjectFactory::WithShader(Shader shader) {
	object->SetShader(shader);
	return this;
}

/*
Sets the position of the scene object
@param position - The position where to place the object
*/
ObjectFactory* ObjectFactory::WithPosition(const glm::vec3& position) {
	object->Translate(position);
	return this;
}

/*
Sets the rotation of the scene object
@param angle - The angle (in radians) of rotation
@param axis - The axis on which to rotate
*/
ObjectFactory* ObjectFactory::WithRotation(const float angle, const glm::vec3& axis) {
	object->Rotate(angle, axis);
	return this;
}

/*
Sets the scale of the scene object
@param scale - The scale of the object. (1, 1, 1) is no difference
*/
ObjectFactory* ObjectFactory::WithScale(const glm::vec3& scale) {
	object->Scale(scale);
	return this;
}

/*
Sets the animation of the scene object
@param animation - A pointer to the heap allocated animation
*/
ObjectFactory* ObjectFactory::WithAnimation(Animation* animation) {
	object->SetAnimation(animation);
	return this;
}

/*
Returns the object pointer to work with
*/
SceneObject* ObjectFactory::Build() {
	return object;
}