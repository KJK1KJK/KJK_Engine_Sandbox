#include "Shader.h"

#include <KJK>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
	//Create a shader program
	ID = glCreateProgram();

	//Load and compile the vertex shader
	GLuint vertex = loadAndCompileShader(vertexPath, GL_VERTEX_SHADER);
	//Load and compile the fragment shader
	GLuint fragment = loadAndCompileShader(fragmentPath, GL_FRAGMENT_SHADER);

	//Attach the shaders to the program
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	//Link the shader program
	glLinkProgram(ID);

	//Check for linking errors
	GLint programLinked = GL_TRUE;
	glGetProgramiv(ID, GL_LINK_STATUS, &programLinked);
	if (programLinked != GL_TRUE)
	{
		KJK_ERROR("Error linking program {0}!", ID);
		printProgramLog(ID);
	}

	//Delete the shaders as they're linked into the program now and are no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::Shader(const GLchar* vertexPath, const GLchar* geometryPath, const GLchar* fragmentPath)
{
	//Create a shader program
	ID = glCreateProgram();

	//Load and compile the vertex shader
	GLuint vertex = loadAndCompileShader(vertexPath, GL_VERTEX_SHADER);
	//Load and compile the geometry shader
	GLuint geometry = loadAndCompileShader(geometryPath, GL_GEOMETRY_SHADER);
	//Load and compile the fragment shader
	GLuint fragment = loadAndCompileShader(fragmentPath, GL_FRAGMENT_SHADER);

	//Attach the shaders to the program
	glAttachShader(ID, vertex);
	glAttachShader(ID, geometry);
	glAttachShader(ID, fragment);
	//Link the shader program
	glLinkProgram(ID);

	//Check for linking errors
	GLint programLinked = GL_TRUE;
	glGetProgramiv(ID, GL_LINK_STATUS, &programLinked);
	if (programLinked != GL_TRUE)
	{
		KJK_ERROR("Error linking program {0}!", ID);
		printProgramLog(ID);
	}

	//Delete the shaders as they're linked into the program now and are no longer necessary
	glDeleteShader(vertex);
	glDeleteShader(geometry);
	glDeleteShader(fragment);
}

Shader::~Shader()
{
	if (ID != 0)
	{
		glDeleteProgram(ID);
		ID = 0;
	}
}

Shader::Shader(Shader&& other) noexcept
{
	//Transfer ownership of the shader program ID
	ID = other.ID;
	other.ID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
	if(this != &other)
	{
		//Delete the existing shader program
		if (ID != 0)
		{
			glDeleteProgram(ID);
		}
		//Transfer ownership of the shader program ID
		ID = other.ID;
		other.ID = 0;
	}

	return *this;
}

//Use the shader program
void Shader::Use() const
{
	glUseProgram(ID);
}

//Set a boolean uniform variable in the shader
void Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

//Set an integer uniform variable in the shader
void Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

//Set a float uniform variable in the shader
void Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& vec) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& vec) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vec));
}

//Prints out the shader log for a shader object
void Shader::PrintShaderLog(GLuint shader)
{
	//Make sure name is shader
	if (glIsShader(shader))
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log and print it
		glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			KJK_ERROR("{0}", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		KJK_ERROR("Name {0} is not a shader", shader);
	}
}

//Prints out the program log for a program object
void Shader::printProgramLog(GLuint program)
{
	//Make sure name is shader
	if (glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//Allocate string
		char* infoLog = new char[maxLength];

		//Get info log and print it
		glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0)
		{
			KJK_ERROR("{0}", infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		KJK_ERROR("Name {0} is not a program", program);
	}
}

GLuint Shader::loadAndCompileShader(const GLchar* shaderPath, GLenum type)
{
	//Declare variables for reading the shader file
	std::string code;
	std::ifstream shaderFile;
	//Ensure ifstream object can throw exceptions
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		//Open the shader file
		shaderFile.open(shaderPath);

		//Declare a string stream to hold shader code
		std::stringstream gShaderStream;

		//Read file's buffer contents into streams
		gShaderStream << shaderFile.rdbuf();

		//Close the file handler
		shaderFile.close();

		//Convert the stream into a string
		code = gShaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		KJK_ERROR("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: {0}", e.what());
	}

	//Convert the string to a GLchar pointer
	const GLchar* shaderCode = code.c_str();

	//Declare the shader ID variable
	GLuint shader{ 0 };

	//Create and compile a geometry shader
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);

	//Check for geometry shader compile errors
	GLint shaderCompiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE)
	{
		KJK_ERROR("Unable to compile the shader {0}!", shader);
		PrintShaderLog(shader);
	}

	//Return the shader Id
	return shader;
}
