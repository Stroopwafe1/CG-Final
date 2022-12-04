#pragma once
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <vector>
#include "Material.h"
#include "LightSource.h"
#include "Shader.h"
#include "Animation.h"

class SceneObject {
public:
	const char* Name; // The name of the scene object
	glm::vec3 m_Position = glm::vec3(), m_Rotation = glm::vec3(); // The absolute position and rotation of the scene object
private:
	GLuint m_Texture_ID, m_Vao; // The texture ID and Vertex Array Object
	GLuint m_Programme_ID; // The program ID made with the vertex and fragment shader
	GLuint uniform_mv; // The uniform model-view variable
	GLuint uniform_material_ambient; // The uniform material ambient_colour variable
	GLuint uniform_material_diffuse; // The uniform material diffuse_colour variable
	GLuint uniform_material_specular; // The uniform material specular variable
	GLuint uniform_material_power; // The uniform material specular power variable
	GLuint uniform_light_pos; // The uniform light position variable
	const Material* m_Material; // A pointer to the given material
	const LightSource* m_Light; // A pointer to the given light
	Animation* m_Animation; // A pointer to the given animation
	const char* m_FragmentShader; // The compiled fragment shader
	const char* m_VertexShader; // The compiled vertex shader
	glm::mat4 m_Model, m_MV; // The model matrix and model-view matrix
	std::vector<glm::vec3> m_Normals[1], m_Vertices[1]; // The vector array of normals and vertices
	std::vector<glm::vec2> m_UVs[1]; // The vector array of UVs
	Shader m_Shader; // The shader type of this object

public:
	// Methods are documented in SceneObject.cpp
	SceneObject();
	SceneObject(const char* name, const char* modelPath, const char* texturePath, Shader shader);
	~SceneObject();
	void LoadModel(const char* modelPath);
	void LoadTexture(const char* texturePath);
	void SetShader(Shader shader);
	void SetShader(const char* fragmentShaderPath, const char* vertexShaderPath);
	Shader GetShader();
	void SetMaterial(const Material* material);
	void SetLight(const LightSource* lightsource);
	void Render(const glm::mat4* view);
	void InitBuffers(const glm::mat4* view, const glm::mat4* projection);
	void Translate(const glm::vec3& translation);
	void Rotate(const float angleRad, const glm::vec3& axis);
	void Scale(const glm::vec3& scalar);
	void SetAnimation(Animation* animation);
	void ClearAnimation();
	void Animate();
};
