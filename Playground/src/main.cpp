//File for testing external library features before implementing them in the engine
#include <KJK>

#include <glad/glad.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

//Initializes the logging system
void initLogger();
//Initializes OpenGl and SDL, then creates a window
bool init();
//Cleans up and closes SDL and all used objects
void close();
//Initializes OpenGL
bool initGL();
//Print shader log
void printShaderLog(GLuint shader);
//Print program log
void printProgramLog(GLuint program);

//Global variables
constexpr int SCREEN_WIDTH{ 800 };
constexpr int SCREEN_HEIGHT{ 600 };

//The window to render to
SDL_Window* gWindow = nullptr;

//Context for OpenGL
SDL_GLContext gContext;

//OpenGl object IDs
GLuint gVBOs[2];
GLuint gVAOs[2];
GLuint gEBOs[2];

//Shader program ID
GLuint gShaderPrograms[2];

//The main function
int main(int argc, char* args[])
{
	//Initialize the logging system
	initLogger();

	//Final exit code
	int exitCode{ 0 };

	//Initialize SDL and OpenGL, then create a window
	if (init() == false)
	{
		KJK_ERROR("Failed to initialize SDL and OpenGL!");
		close();
		exitCode = 1;
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_EVENT_QUIT)
				{
					quit = true;
				}
				//Window resize event
				if(e.type == SDL_EVENT_WINDOW_RESIZED)
				{
					//Adjust the viewport when the window size changes
					glViewport(0, 0, e.window.data1, e.window.data2);
				}
				//Handle keyboard input
				else if (e.type == SDL_EVENT_KEY_DOWN)
				{
					switch (e.key.key)
					{
					case SDLK_ESCAPE:
						quit = true;
						break;
					default:
						break;
					}
				}
			}

			//Set the clear color
			glClearColor(1.f, 1.f, 1.f, 1.0f);
			//Clear the color buffer
			glClear(GL_COLOR_BUFFER_BIT);

			//Set to polygon wireframe mode
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			//Use the defined shader program
			glUseProgram(gShaderPrograms[0]);

			//Bind the first VAO
			glBindVertexArray(gVAOs[0]);

			//Draw a square
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			//Use the defined shader program
			glUseProgram(gShaderPrograms[1]);

			//Bind the second VAO
			glBindVertexArray(gVAOs[1]);

			//Draw a triangle
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

			//Unbind the VAO
			glBindVertexArray(0);

			//Update screen
			SDL_GL_SwapWindow(gWindow);
		}
	}

	//Clean up and close SDL and all used objects
	close();

	return exitCode;
}

//Initializes the logging system
static void initLogger()
{
	KJK::Logger::Init();
	KJK_INFO("Started the Playground!");
}

//Initializes OpenGl and SDL, then creates a window
bool init()
{
	//Initlialize flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) == false)
	{
		KJK_ERROR("Failed to initialize SDL: {0}", SDL_GetError());
		success = false;
	}
	else
	{
		//Use OpenGL 4.5 core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Create a window
		gWindow = SDL_CreateWindow("LearnOpenGL", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
		if (gWindow == NULL)
		{
			KJK_ERROR("Failed to create window: {0}", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				KJK_ERROR("Failed to create OpenGL context: {0}", SDL_GetError());
				success = false;
			}
			else
			{
				//Load OpenGL functions using GLAD
				if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
				{
					KJK_ERROR("Failed to initialize GLAD! SDL error: {0}", SDL_GetError());
					success = false;
				}
				else
				{
					if(!initGL())
					{
						KJK_ERROR("Failed to initialize OpenGL!");
						success = false;
					}
				}
			}
		}
	}

	return success;
}

//Initialize OpenGL
bool initGL()
{
	//Success flag
	bool success = true;

	//Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	//Generate VAOs
	glGenVertexArrays(2, gVAOs);
	//Generate VBOs
	glGenBuffers(2, gVBOs);
	//Generate EBOs
	glGenBuffers(2, gEBOs);

	//Create a vertex shader source
	const GLchar* vertexShaderSource =
	{
		"#version 450 core\n"
		"layout (location = 0) in vec3 APos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(APos.x, APos.y, APos.z, 1.0);\n"
		"}\0"
	};

	//Create a vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Attach the shader source to the shader object
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	//Compile the vertex shader
	glCompileShader(vertexShader);

	//Check for vertex shader compile errors
	GLint shaderCompiled = GL_FALSE;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE)
	{
		KJK_ERROR("Unable to compile vertex shader {0}!", vertexShader);
		printShaderLog(vertexShader);
		success = false;
	}
	else
	{
		//Create a fragment shader source to color a square orange
		const GLchar* fragmentShaderSource =
		{
			"#version 450 core\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
			"}\0"
		};

		//Create a fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		//Attach the shader source to the shader object
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		//Compile the fragment shader
		glCompileShader(fragmentShader);

		//Check for fragment shader compile errors
		shaderCompiled = GL_FALSE;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
		if (shaderCompiled != GL_TRUE)
		{
			KJK_ERROR("Unable to compile fragment shader {0}!", fragmentShader);
			printShaderLog(fragmentShader);
			success = false;
		}
		else
		{
			//Create shader program
			gShaderPrograms[0] = glCreateProgram();

			//Attach shaders to the program
			glAttachShader(gShaderPrograms[0], vertexShader);
			glAttachShader(gShaderPrograms[0], fragmentShader);
			glLinkProgram(gShaderPrograms[0]);

			//Check for linking errors
			GLint programLinked = GL_TRUE;
			glGetProgramiv(gShaderPrograms[0], GL_LINK_STATUS, &programLinked);
			if (programLinked != GL_TRUE)
			{
				KJK_ERROR("Error linking program {0}!", gShaderPrograms[0]);
				printProgramLog(gShaderPrograms[0]);
				success = false;
			}
			else
			{
				//Delete the fragment shader as it's no longer needed
				glDeleteShader(fragmentShader);

				//Set of vertices for a square
				GLfloat vertices[] =
				{
					//Square
					 0.5f,  0.5f, 0.0f,  //Top Right
					 0.5f, -0.5f, 0.0f,  //Bottom Right
					-0.5f, -0.5f, 0.0f,  //Bottom Left
					-0.5f,  0.5f, 0.0f  //Top Left
				};
				GLuint indices[] =
				{
					0, 1, 3,  //First Triangle
					1, 2, 3  //Second Triangle
				};

				//Bind VAO
				glBindVertexArray(gVAOs[0]);

				//Bind VBO
				glBindBuffer(GL_ARRAY_BUFFER, gVBOs[0]);
				//Fill VBO with vertex data
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

				//Bind EBO
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBOs[0]);
				//Fill EBO with index data
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

				//Inform OpenGL how to interpret the vertex data
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
			}
		}

		//Create a fragment shader source to color a triangle yellow
		fragmentShaderSource =
		{
			"#version 450 core\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
			"}\0"
		};

		//Create a fragment shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		//Attach the shader source to the shader object
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		//Compile the fragment shader
		glCompileShader(fragmentShader);

		//Check for fragment shader compile errors
		shaderCompiled = GL_FALSE;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
		if (shaderCompiled != GL_TRUE)
		{
			KJK_ERROR("Unable to compile fragment shader {0}!", fragmentShader);
			printShaderLog(fragmentShader);
			success = false;
		}
		else
		{
			//Create shader program
			gShaderPrograms[1] = glCreateProgram();

			//Attach shaders to the program
			glAttachShader(gShaderPrograms[1], vertexShader);
			glAttachShader(gShaderPrograms[1], fragmentShader);
			glLinkProgram(gShaderPrograms[1]);

			//Check for linking errors
			GLint programLinked = GL_TRUE;
			glGetProgramiv(gShaderPrograms[1], GL_LINK_STATUS, &programLinked);
			if (programLinked != GL_TRUE)
			{
				KJK_ERROR("Error linking program {0}!", gShaderPrograms[1]);
				printProgramLog(gShaderPrograms[1]);
				success = false;
			}
			else
			{
				//Set of vertices for a traingle
				GLfloat vertices[] =
				{
					 0.9f,  0.9f, 0.0f,   //Top Right
					 0.9f,  0.7f, 0.0f,   //Bottom
					 0.7f,  0.9f, 0.0f    //Top Left
				};
				GLuint indices[] =
				{
					0, 1, 2
				};

				//Bind VAO
				glBindVertexArray(gVAOs[1]);

				//Bind VBO
				glBindBuffer(GL_ARRAY_BUFFER, gVBOs[1]);
				//Fill VBO with vertex data
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

				//Bind EBO
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBOs[1]);
				//Fill EBO with index data
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

				//Inform OpenGL how to interpret the vertex data
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
			}
		}
		
	}

	return success;
}


//Cleans up and closes SDL and all used objects
void close()
{
	glDeleteVertexArrays(2, gVAOs);
	glDeleteBuffers(2, gVBOs);
	glDeleteProgram(gShaderPrograms[0]);
	glDeleteProgram(gShaderPrograms[1]);
	
	//Destroy window
	if (gWindow != nullptr)
	{
		SDL_DestroyWindow(gWindow);
		gWindow = nullptr;
	}

	//Quit SDL subsystems
	SDL_Quit();
}

//Prints out the shader log for a shader object
void printShaderLog( GLuint shader )
{
	//Make sure name is shader
	if( glIsShader( shader ) )
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		//Get info string length
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
		
		//Allocate string
		char* infoLog = new char[ maxLength ];
		
		//Get info log and print it
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
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
void printProgramLog( GLuint program )
{
	//Make sure name is shader
	if(glIsProgram(program))
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		//Get info string length
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
		
		//Allocate string
		char* infoLog = new char[ maxLength ];
		
		//Get info log and print it
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
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