#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "SceneObject.h"
#include "objloader.h"
#include "texture.h"
#include "glsl.h"
#include "MathsHelper.h"

/*
Constructor used for the factory.
!!!DO NOT CALL THIS MANUALLY!!!
*/
SceneObject::SceneObject() {
	m_Model = glm::mat4(1.0f);
	m_Animation = nullptr;
}

/*
Constructor used for manual creation.
Loads the object and texture.
*/
SceneObject::SceneObject(const char* name, const char* modelPath, const char* texturePath, Shader shader) {
	bool res = loadOBJ(modelPath, m_Vertices[0], m_UVs[0], m_Normals[0]);
	m_Texture_ID = loadBMP(texturePath);
	m_Model = glm::mat4(1.0f);
	Name = name;
	m_Shader = shader;
	m_Animation = nullptr;
}

/*
Destructor, handles cleanup of the animation if there is any
*/
SceneObject::~SceneObject() {
	if (m_Animation != nullptr)
		delete m_Animation;
}

/*
Loads the object file
@param modelPath - The path of the .obj file
*/
void SceneObject::LoadModel(const char* modelPath) {
	bool res = loadOBJ(modelPath, m_Vertices[0], m_UVs[0], m_Normals[0]);
}

/*
Loads the texture file
@param texturePath - The path of the .bmp file
*/
void SceneObject::LoadTexture(const char* texturePath) {
	m_Texture_ID = loadBMP(texturePath);
}

/*
Sets the shader type of the object
@param shader - The shader type
*/
void SceneObject::SetShader(Shader shader) {
	m_Shader = shader;
}

/*
Sets the vertex shader and fragment shader for this object
@param fragmentShaderPath - The path of the fragment shader
@param vertexShaderPath - The path of the vertex shader
*/
void SceneObject::SetShader(const char* fragmentShaderPath, const char* vertexShaderPath) {
	m_VertexShader = glsl::readFile(vertexShaderPath);
	GLuint vsh_id = glsl::makeVertexShader(m_VertexShader);

	m_FragmentShader = glsl::readFile(fragmentShaderPath);
	GLuint fsh_id = glsl::makeFragmentShader(m_FragmentShader);

	m_Programme_ID = glsl::makeShaderProgram(vsh_id, fsh_id);
}

/*
Get what shader type this object is
@returns The shader type
*/
Shader SceneObject::GetShader() {
	return m_Shader;
}

/*
Sets the material of this object
@param material - The material
*/
void SceneObject::SetMaterial(const Material* material) {
	m_Material = material;
}

/*
Sets the light source of this object
@param lightsource - The light source
*/
void SceneObject::SetLight(const LightSource* lightsource) {
	m_Light = lightsource;
}

/*
Renders the object to the screen
@param view - The view matrix
*/
void SceneObject::Render(const glm::mat4* view) {
	glUseProgram(m_Programme_ID);

	m_MV = *view * m_Model;

	// Send mv
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, glm::value_ptr(m_MV));

	glBindTexture(GL_TEXTURE_2D, m_Texture_ID);

	glUniform3fv(uniform_light_pos, 1, glm::value_ptr((*m_Light).position));
	glUniform3fv(uniform_material_ambient, 1, glm::value_ptr((*m_Material).ambient_colour));
	glUniform3fv(uniform_material_diffuse, 1, glm::value_ptr((*m_Material).diffuse_colour));
	glUniform3fv(uniform_material_specular, 1, glm::value_ptr((*m_Material).specular));
	glUniform1f(uniform_material_power, (*m_Material).power);

	// Send vao
	glBindVertexArray(m_Vao);
	glDrawArrays(GL_TRIANGLES, 0, m_Vertices[0].size());
	glBindVertexArray(0);
}

/*
Initialises the buffers of the object
@param view - The view matrix
@param projection - The projection matrix
*/
void SceneObject::InitBuffers(const glm::mat4* view, const glm::mat4* projection) {
	GLuint position_id, color_id;
	GLuint vbo_vertices, vbo_normals, vbo_uvs;

	GLuint uv_id = glGetAttribLocation(m_Programme_ID, "uv");

	// Get vertex attributes
	position_id = glGetAttribLocation(m_Programme_ID, "position");
	color_id = glGetAttribLocation(m_Programme_ID, "colour");
	GLuint normal_id = glGetAttribLocation(m_Programme_ID, "normal");

	// Make uniform vars
	uniform_mv = glGetUniformLocation(m_Programme_ID, "mv");
	GLuint uniform_proj = glGetUniformLocation(m_Programme_ID, "projection");
	uniform_light_pos = glGetUniformLocation(m_Programme_ID, "light_pos");
	uniform_material_ambient = glGetUniformLocation(m_Programme_ID, "mat_ambient");
	uniform_material_diffuse = glGetUniformLocation(m_Programme_ID, "mat_diffuse");
	uniform_material_specular = glGetUniformLocation(m_Programme_ID, "mat_specular");
	uniform_material_power = glGetUniformLocation(m_Programme_ID, "mat_power");

	glGenBuffers(1, &vbo_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glBufferData(GL_ARRAY_BUFFER,
		m_Normals[0].size() * sizeof(glm::vec3),
		&m_Normals[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER,
		m_Vertices[0].size() * sizeof(glm::vec3), &m_Vertices[0][0],
		GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_uvs);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
	glBufferData(GL_ARRAY_BUFFER, m_UVs[0].size() * sizeof(glm::vec2),
		&m_UVs[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Allocate memory for vao
	glGenVertexArrays(1, &m_Vao);

	// Bind to vao
	glBindVertexArray(m_Vao);

	// Bind vertices to vao
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(position_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
	glVertexAttribPointer(normal_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(normal_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_uvs);
	glVertexAttribPointer(uv_id, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(uv_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Stop bind to vao
	glBindVertexArray(0);
	// Define model
	m_MV = *view * m_Model;

	// Send mvp
	glUseProgram(m_Programme_ID);
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, glm::value_ptr(m_MV));
	glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(*projection));
}

/*
Translate (or move) the object with the specified translation
There is a check on the rotation of the object because it would not translate in the way you would expect
Also updates the m_Position variable to keep track of the absolute position of the object
@param translation - The translation to apply to this object
*/
void SceneObject::Translate(const glm::vec3& translation) {
	glm::vec3 trans = translation;
	if (m_Rotation != glm::vec3(0.0f)) {
		if (m_Rotation.y <= 90) {
			trans.z = translation.x;
			trans.x = translation.z;
		} else if (m_Rotation.y <= 180) {
			trans = -translation;
		} else if (m_Rotation.y <= 270) {
			trans.z = -translation.x;
			trans.x = -translation.z;
		} else {
			trans = translation;
		}
	}

	m_Model = glm::translate(m_Model, trans);
	m_Position += translation;
}

/*
Rotate the object with given angle on the given axis
Also updates the m_Rotation variable to keep track of the absolute rotation of the object (in degrees)
@param angleRad - The angle to rotate with (in radians)
@param axis - The axis to rotate on. (0, 1, 0) would rotate on the y-axis and stay horizontal
*/
void SceneObject::Rotate(const float angleRad, const glm::vec3& axis) {
	m_Model = glm::rotate(m_Model, angleRad, axis);
	m_Rotation += glm::degrees(angleRad) * axis;
	if (m_Rotation.x < 0)
		m_Rotation.x += 360;
	if (m_Rotation.y < 0)
		m_Rotation.y += 360;
	if (m_Rotation.z < 0)
		m_Rotation.z += 360;
	m_Rotation.x = (int)m_Rotation.x % 360;
	m_Rotation.y = (int)m_Rotation.y % 360;
	m_Rotation.z = (int)m_Rotation.z % 360;
}

/*
Scales the object with a given scalar
@param scalar - The scalar to apply. (1, 1, 1) would be unchanged
*/
void SceneObject::Scale(const glm::vec3& scalar) {
	m_Model = glm::scale(m_Model, scalar);
}

/*
Sets the animation to play from the heap-allocated animation variable
@param animation - The animation to play
*/
void SceneObject::SetAnimation(Animation* animation) {
	m_Animation = animation;
}

/*
Clears the animation if one was set
*/
void SceneObject::ClearAnimation() {
	if (m_Animation != nullptr) {
		delete m_Animation;
		m_Animation = nullptr;
	}
}

/*
Animates the object if one was set
*/
void SceneObject::Animate() {
	if (m_Animation != nullptr) {
		m_Animation->Animate(this);
	}
}