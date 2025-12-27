#pragma once

class Shader
{
public:
	//Program ID
	GLuint ID;

	//Constructor reads and builds the shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	~Shader();

	//Disable copy semantics
	Shader(const Shader& other) = delete;
	Shader& operator=(const Shader& other) = delete;

	//Allow move semantics
	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	//Use the shader program
	void Use() const;

	//Set a boolean uniform variable in the shader
	void SetBool(const std::string& name, bool value) const;
	//Set an integer uniform variable in the shader
	void SetInt(const std::string& name, int value) const;
	//Set a float uniform variable in the shader
	void SetFloat(const std::string& name, float value) const;
	//Set a matrix4 uniform variable in the shader
	void SetMat4(const std::string& name, const glm::mat4& mat) const;

private:
	//Prints out the shader log for a shader object
	void PrintShaderLog(GLuint shader);
	//Prints out the program log for a program object
	void printProgramLog(GLuint program);
};

