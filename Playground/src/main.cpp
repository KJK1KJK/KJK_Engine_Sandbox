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

//Global variables
constexpr int SCREEN_WIDTH{ 800 };
constexpr int SCREEN_HEIGHT{ 600 };

//The window to render to
SDL_Window* gWindow = nullptr;

//Context for OpenGL
SDL_GLContext gContext;

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
					//Define the viewport dimensions
					glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				}
			}
		}
	}

	return success;
}

//Cleans up and closes SDL and all used objects
void close()
{
	//Destroy window
	if (gWindow != nullptr)
	{
		SDL_DestroyWindow(gWindow);
		gWindow = nullptr;
	}

	//Quit SDL subsystems
	SDL_Quit();
}