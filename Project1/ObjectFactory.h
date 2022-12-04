#pragma once
#include "SceneObject.h"
class ObjectFactory {
private:
	SceneObject* object; // The object to work with

public:
	//Methods documented in ObjectFactory.cpp
	ObjectFactory();
	// No destructor because the cleanup is handled by the Cleanup() function in main.cpp
	ObjectFactory* New();
	ObjectFactory* WithName(const char* name);
	ObjectFactory* FromObjectModel(const char* objFilePath);
	ObjectFactory* WithTexture(const char* bmpFilePath);
	ObjectFactory* WithShader(Shader shader);
	ObjectFactory* WithPosition(const glm::vec3& position);
	ObjectFactory* WithRotation(const float angle, const glm::vec3& axis);
	ObjectFactory* WithScale(const glm::vec3& scale);
	ObjectFactory* WithAnimation(Animation* animation);
	SceneObject* Build();
};
