//File for testing external library features before implementing them in the engine
#include <KJK>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <SDL3/SDL_main.h>

//Initializes the logging system
void initLogger();
//Initializes OpenGl and SDL, then creates a window
bool init();
//Initializes OpenGL
bool initGL();
//Loads media
bool loadMedia();
//Cleans up and closes SDL and all used objects
void close();

//Global variables
int SCREEN_WIDTH{ 800 };
int SCREEN_HEIGHT{ 600 };

//The window to render to
SDL_Window* gWindow = nullptr;

//Context for OpenGL
SDL_GLContext gContext;

//Shader program ID
std::optional<std::array<Shader, 3>> gShaders;

//Camera object
Camera* gCamera;

//Light color and position struct
struct Light
{
	glm::vec3 position;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

//Directional light object
Light gDirectionalLight;
//Point light object
Light gSpotLight;

//Model object
Model* gModel;

//The main function
int main(int argc, char* args[])
{
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
		//Load media
		if(loadMedia() == false)
		{
			KJK_ERROR("Failed to load media!");
			exitCode = 2;
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;
			SDL_zero(e);

			//Time variables
			float deltaTime{ 0.0f };
			float lastFrame{ 0.0f };

			//Set the initial mix value for texture blending
			float mixValue = 0.2f;
			(*gShaders)[0].Use();
			(*gShaders)[0].SetFloat("mixValue", mixValue);

			//Mouse Input mode setting
			bool mouseCaptured = true;

			//Enable movement setting
			bool enableMovement = true;

			//Flashlight on/off setting
			bool flashlightEnabled = true;

			//Initialize time value
			float timeValue = 0.0f;

			//Enum for input state control
			enum class InputState
			{
				NONE,
				MIX,
				FOV
			} inputState{InputState::NONE};

			//While application is running
			while (!quit)
			{
				//Calculate delta time
				float currentFrame = static_cast<float>(SDL_GetTicks()) / 1000.0f;
				deltaTime = currentFrame - lastFrame;
				lastFrame = currentFrame;

				//Get keyboard state
				const bool* keyState = SDL_GetKeyboardState(NULL);

				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_EVENT_QUIT)
					{
						quit = true;
					}
					//Window resize event
					else if (e.type == SDL_EVENT_WINDOW_RESIZED)
					{
						//Adjust the viewport when the window size changes
						glViewport(0, 0, e.window.data1, e.window.data2);

						//Adjust the screen width and height variables
						SCREEN_WIDTH = e.window.data1;
						SCREEN_HEIGHT = e.window.data2;
					}
					//Handle keyboard input
					else if (e.type == SDL_EVENT_KEY_DOWN)
					{
						switch (e.key.key)
						{
						case SDLK_ESCAPE: //Exit the application
							quit = true;
							break;
						case SDLK_1: //Enable mix value control
							inputState = InputState::MIX;
							break;
						case SDLK_2: //Enable FoV control
							inputState = InputState::FOV;
							break;
						case SDLK_M: //Switch mouse capture mode
							mouseCaptured = !mouseCaptured;
							SDL_SetWindowRelativeMouseMode(gWindow, mouseCaptured);
							break;
						case SDLK_P: //Switch object movement setting
							enableMovement = !enableMovement;
							break;
						case SDLK_F: //Toggle flashlight
							flashlightEnabled = !flashlightEnabled;

							(*gShaders)[0].Use();
							if(flashlightEnabled)
							{
								(*gShaders)[0].SetVec3("spotLight.ambient", gSpotLight.ambient);
								(*gShaders)[0].SetVec3("spotLight.diffuse", gSpotLight.diffuse);
								(*gShaders)[0].SetVec3("spotLight.specular", gSpotLight.specular);
							}
							else
							{
								(*gShaders)[0].SetVec3("spotLight.ambient", glm::vec3(0.0f));
								(*gShaders)[0].SetVec3("spotLight.diffuse", glm::vec3(0.0f));
								(*gShaders)[0].SetVec3("spotLight.specular", glm::vec3(0.0f));
							}
							break;
						case SDLK_UP: //Increase the appropriate value
							switch (inputState)
							{
							case InputState::MIX: //Increase the mix value
								mixValue += 0.05f;
								if (mixValue > 1.0f)
								{
									mixValue = 1.0f;
								}
								//Apply the new mix value
								(*gShaders)[0].Use();
								(*gShaders)[0].SetFloat("mixValue", mixValue);
								break;
							case InputState::FOV: //Increase the FoV
								gCamera->fov += 5.0f;
								if (gCamera->fov > 360.0f)
								{
									gCamera->fov = 360.0f;
								}
								break;
							default:
								break;
							}
							break;
						case SDLK_DOWN: //Decrease the appropriate value
							switch (inputState)
							{
							case InputState::MIX:
								mixValue -= 0.05f;
								if (mixValue < 0.0f)
								{
									mixValue = 0.0f;
								}
								//Apply the new mix value
								(*gShaders)[0].Use();
								(*gShaders)[0].SetFloat("mixValue", mixValue);
								break;
							case InputState::FOV:
								gCamera->fov -= 5.0f;
								if (gCamera->fov < 0.0f)
								{
									gCamera->fov = 0.0f;
								}
								break;
							default:
								break;
							}
							break;
						default:
							break;
						}
					}

					//Handle camera mouse and keyboard input
					gCamera->HandleInput(e, deltaTime, mouseCaptured, nullptr);
				}

				//Handle camera keystate input
				gCamera->HandleInput(SDL_Event{}, deltaTime, mouseCaptured, keyState);

				//Set the clear color
				glClearColor(0.2f, 0.f, 0.2f, 1.0f);
				//Clear the color buffer
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//Set to polygon wireframe mode
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				//Calculate the time value
				if (enableMovement)
				{
					timeValue += deltaTime;
				}

				//Enable the shader for the model
				(*gShaders)[0].Use();
				
				//Define a view matrix
				glm::mat4 view = gCamera->GetViewMatrix();

				//Define a projection matrix
				glm::mat4 projection = glm::mat4(1.0f);
				projection = glm::perspective(glm::radians(gCamera->fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

				//Update the spotlight position and direction uniforms
				(*gShaders)[0].Use();
				(*gShaders)[0].SetVec3("spotLight.position", gCamera->position);
				(*gShaders)[0].SetVec3("spotLight.direction", gCamera->direction);

				//Update the view and projection matrices in the shader
				(*gShaders)[0].SetMat4("view", view);
				(*gShaders)[0].SetMat4("projection", projection);

				//Define the model matrix
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
				//Scale the model down
				model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

				//Update the model matrix in the shader
				(*gShaders)[0].SetMat4("model", model);

				//Render the model
				gModel->Draw((*gShaders)[0]);

				//Update screen
				SDL_GL_SwapWindow(gWindow);
			}
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
	//Initialize flag
	bool success = true;

	//Initialize the logging system
	initLogger();

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) == NULL)
	{
		KJK_ERROR("Failed to initialize SDL: {0}", SDL_GetError());
		success = false;
	}
	else
	{
		KJK_INFO("Initialized SDL!");

		//Use OpenGL 4.5 core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		//Create a window
		gWindow = SDL_CreateWindow("LearnOpenGL", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (gWindow == NULL)
		{
			KJK_ERROR("Failed to create window: {0}", SDL_GetError());
			success = false;
		}
		else
		{
			KJK_INFO("Created an SDL window!");

			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				KJK_ERROR("Failed to create OpenGL context: {0}", SDL_GetError());
				success = false;
			}
			else
			{
				KJK_INFO("Created an SDL context!");

				//Set the mouse mode to relative
				SDL_SetWindowRelativeMouseMode(gWindow, true);

				//Initialize the camera
				gCamera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

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

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);

	//Define the directional light properties
	gDirectionalLight.position = glm::vec3(0.0f, 0.0f, 0.0f);
	gDirectionalLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	gDirectionalLight.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	gDirectionalLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);

	//Define the spotlight properties
	gSpotLight.position = glm::vec3(0.0f, 0.0f, 0.0f);
	gSpotLight.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	gSpotLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	gSpotLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);

	KJK_INFO("Initialized OpenGL!");

	//Return the success flag
	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Create the shaders
	gShaders.emplace(std::array<Shader, 3>
	{
		Shader("assets/shaders/shader.vert", "assets/shaders/backpackShader.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/shader2.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/lightSourceShader.frag")
	});

	//Load the model from a file
	gModel = new Model("assets/backpack/backpack.obj");

	//Use the first shader program
	(*gShaders)[0].Use();

	//Set color uniforms for the spotlight
	(*gShaders)[0].SetVec3("spotLight.ambient", gSpotLight.ambient);
	(*gShaders)[0].SetVec3("spotLight.diffuse", gSpotLight.diffuse);
	(*gShaders)[0].SetVec3("spotLight.specular", gSpotLight.specular);
	//Set spotlight cutoff angles
	(*gShaders)[0].SetFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	(*gShaders)[0].SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));
	//Set spotlight attenuation factors
	(*gShaders)[0].SetFloat("spotLight.constant", 1.0f);
	(*gShaders)[0].SetFloat("spotLight.linear", 0.045f);
	(*gShaders)[0].SetFloat("spotLight.quadratic", 0.0075f);

	//Set color uniforms for the directional light
	(*gShaders)[0].SetVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
	(*gShaders)[0].SetVec3("dirLight.ambient", gDirectionalLight.ambient);
	(*gShaders)[0].SetVec3("dirLight.diffuse", gDirectionalLight.diffuse);
	(*gShaders)[0].SetVec3("dirLight.specular", gDirectionalLight.specular);

	KJK_INFO("Loaded media!");

	//Return the success flag
	return success;
}

//Cleans up and closes SDL and all used objects
void close()
{
	//Delete the camera
	delete gCamera;

	//Delete the model
	delete gModel;

	//Destroy window
	if (gWindow != nullptr)
	{
		SDL_DestroyWindow(gWindow);
		gWindow = nullptr;
	}

	//Quit SDL subsystems
	SDL_Quit();

	KJK_INFO("Exited the application!");
}