#include "Shader.h"

#include <KJK>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
	//Variables necessary for reading shader files
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	//Ensure ifstream objects can throw exceptions
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		//Open the shader files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);

		//Declare string streams to hold shader code
		std::stringstream vShaderStream, fShaderStream;

		//Read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		//Close the file handlers
		vShaderFile.close();
		fShaderFile.close();

		//Convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		KJK_ERROR("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: {0}", e.what());
	}

	//Convert strings to GLchar pointers
	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();

	//Declare the shader ID variables
	GLuint vertex{ 0 }, fragment{ 0 };

	//Create and compile vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	//Check for vertex shader compile errors
	GLint shaderCompiled = GL_FALSE;
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE)
	{
		KJK_ERROR("Unable to compile vertex shader {0}!", vertex);
		PrintShaderLog(vertex);
	}

	//Create and compile fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	//Check for fragment shader compile errors
	shaderCompiled = GL_FALSE;
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE)
	{
		KJK_ERROR("Unable to compile fragment shader {0}!", fragment);
		PrintShaderLog(fragment);
	}

	//Create a shader program
	ID = glCreateProgram();

	//Attach shaders to the program
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
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