//File for testing external library features before implementing them in the engine
#include <KJK>

#include <SDL3/SDL.h> //Contains the SDL datatypes and functions
#include <SDL3/SDL_main.h> //Defines main for SDL applications

//Screen dimension constants
constexpr int kScreenWidth{ 640 };
constexpr int kScreenHeight{ 480 };

//The window to render to
SDL_Window* gWindow{ nullptr };

//The surface contained by the window
SDL_Surface* gScreenSurface{ nullptr };

//The image to load and show on the screen
SDL_Surface* gHelloWorld{ nullptr };

void initLogger();
bool init();
bool loadMedia();
void close();

int main(int argc, char* args[])
{
	//Initialize the logging system
	initLogger();


	//Final exit code
	int exitCode{ 0 };

	//Initialize SDL
	if (init() == false)
	{
		KJK_ERROR("Failed to initialize!\n");
		exitCode = 1;
	}
	else
	{
		//Load media
		if (loadMedia() == false)
		{
			KJK_ERROR("Failed to load media!\n");
			exitCode = 2;
		}
		else
		{
			//The quit flag
			bool quit{ false };

			//The event data
			SDL_Event e;
			SDL_zero(e);

			//The main loop
			while (quit == false)
			{
				//Handle events on the queue
				while (SDL_PollEvent(&e) == true)
				{
					//Check for quit event
					if (e.type == SDL_EVENT_QUIT)
					{
						quit = true;
					}
				}
				//Fill the surface white
				SDL_FillSurfaceRect(gScreenSurface, nullptr, SDL_MapSurfaceRGB(gScreenSurface, 0xFF, 0xFF, 0xFF));

				//Render the image onto the screen surface
				SDL_BlitSurface(gHelloWorld, nullptr, gScreenSurface, nullptr);

				//Update the surface
				SDL_UpdateWindowSurface(gWindow);
			}
		}
	}

	//Free resources and close SDL
	close();

	return exitCode;
}

//Initializes the logging system
static void initLogger()
{
	KJK::Logger::Init();
	KJK_INFO("Started the Playground!");
}

//Starts up SDL and creates a window
bool init()
{
	//Declare success flag
	bool success{ true };

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) == false)
	{
		KJK_ERROR("SDL could not initialize! SDL_Error: {}\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow("SDL3 Tutorial: Hello SDL3", kScreenHeight, kScreenHeight, 0);
		if(gWindow == nullptr)
		{
			KJK_ERROR("Window could not be created! SDL_Error: {}\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Get window surface
			gScreenSurface = SDL_GetWindowSurface(gWindow);
		}
	}

	return success;
}

//Loads media
bool loadMedia()
{
	//File loading flag
	bool success{ true };

	//Load splash image
	std::string imagePath{ "assets/hello-sdl3.bmp" };
	gHelloWorld = SDL_LoadBMP(imagePath.c_str());
	if( gHelloWorld == nullptr )
	{
		KJK_ERROR("Unable to load image {}! SDL Error: {}\n", imagePath, SDL_GetError());
		success = false;
	}

	return success;
}

//Frees media and shuts down SDL
void close()
{
	//Clean up surface
	SDL_DestroySurface(gHelloWorld);

	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;
	gScreenSurface = nullptr;

	//Quit SDL subsystems
	SDL_Quit();
}