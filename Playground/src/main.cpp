//File for testing external library features before implementing them in the engine
#include <KJK>

#include "Shader.h"

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
std::optional<std::array<Shader, 2>> gShaders;

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
			(*gShaders)[0].Use();

			(*gShaders)[1].SetFloat("uHorizontalOffset", 0.2f);

			//Bind the first VAO
			glBindVertexArray(gVAOs[0]);

			//Draw a square
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

			//Use the defined shader program
			(*gShaders)[1].Use();

			float timeValue = static_cast<float>(SDL_GetTicks()) / 1000.0f;
			float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
			(*gShaders)[1].SetFloat("uGreen", greenValue);

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
	if (SDL_Init(SDL_INIT_VIDEO) == NULL)
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

	//Create two shaders
	gShaders.emplace(std::array<Shader, 2>{
		Shader("assets/shaders/shader.vert", "assets/shaders/shader1.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/shader2.frag")
	});

	//Set of vertices for a triangle
	GLfloat vertices[] =
	{
		//Triangle
		 0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  //Top
		 0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  //Bottom Right
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f   //Bottom Left
	};
	GLuint indices[] =
	{
		0, 1, 2  //First Triangle
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Set of vertices for a traingle
	GLfloat vertices2[] =
	{
		 0.95f,  0.95f, 0.0f,   //Top Right
		 0.95f,  0.55f, 0.0f,   //Bottom
		 0.55f,  0.95f, 0.0f,   //Top Left
	};
	GLuint indices2[] =
	{
		0, 1, 2
	};

	//Bind VAO
	glBindVertexArray(gVAOs[1]);

	//Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, gVBOs[1]);
	//Fill VBO with vertex data
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	//Bind EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBOs[1]);
	//Fill EBO with index data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

	//Inform OpenGL how to interpret the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Return the success flag
	return success;
}


//Cleans up and closes SDL and all used objects
void close()
{
	//Delete OpenGL objects
	glDeleteVertexArrays(2, gVAOs);
	glDeleteBuffers(2, gVBOs);
	glDeleteBuffers(2, gEBOs);
	
	//Destroy window
	if (gWindow != nullptr)
	{
		SDL_DestroyWindow(gWindow);
		gWindow = nullptr;
	}

	//Quit SDL subsystems
	SDL_Quit();
}