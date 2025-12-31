//File for testing external library features before implementing them in the engine
#include <KJK>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "CubeModel.h"
#include "PlaneModel.h"

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
std::optional<std::array<Shader, 5>> gShaders;

//Camera object
Camera* gCamera;

//Light color and position struct
struct Light
{
	glm::vec3 position;

	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

//Directional light object
Light gDirectionalLight;
//Spotlight object
Light gSpotLight;
//Point lights array
std::array<Light, 1> gPointLights;

//Model object
Model* gModel;

//Cube model objects
CubeModel* gCubeModels;

//Plane model object
PlaneModel* gPlaneModel;

//Grass objects
PlaneModel* gGlassPlaneModels;

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
			(*gShaders)[1].Use();
			(*gShaders)[1].SetFloat("mixValue", mixValue);

			//Mouse Input mode setting
			bool mouseCaptured = true;

			//Object movement setting
			bool enableMovement = true;

			//Flashlight on/off setting
			bool flashlightEnabled = false;

			//Highligh outline effect settings
			bool outlineEffectEnabled = false;

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

							(*gShaders)[1].Use();
							if(flashlightEnabled)
							{
								(*gShaders)[1].SetVec4("spotLight.ambient", gSpotLight.ambient);
								(*gShaders)[1].SetVec4("spotLight.diffuse", gSpotLight.diffuse);
								(*gShaders)[1].SetVec4("spotLight.specular", gSpotLight.specular);
							}
							else
							{
								(*gShaders)[1].SetVec4("spotLight.ambient", glm::vec4(0.0f));
								(*gShaders)[1].SetVec4("spotLight.diffuse", glm::vec4(0.0f));
								(*gShaders)[1].SetVec4("spotLight.specular", glm::vec4(0.0f));
							}
							break;
						case SDLK_O: //Toggle outline effect
							outlineEffectEnabled = !outlineEffectEnabled;
							if (outlineEffectEnabled)
							{
								glEnable(GL_STENCIL_TEST);
							}
							else
							{
								glDisable(GL_STENCIL_TEST);
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
								(*gShaders)[1].Use();
								(*gShaders)[1].SetFloat("mixValue", mixValue);
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
								(*gShaders)[1].Use();
								(*gShaders)[1].SetFloat("mixValue", mixValue);
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
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				//Set to polygon wireframe mode
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				//Calculate the time value
				if (enableMovement)
				{
					timeValue += deltaTime;
				}

				//Enable the shader for the model
				(*gShaders)[1].Use();
				
				//Define a view matrix
				glm::mat4 view = gCamera->GetViewMatrix();

				//Define a projection matrix
				glm::mat4 projection = glm::mat4(1.0f);
				projection = glm::perspective(glm::radians(gCamera->fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

				//Update the spotlight position and direction uniforms
				(*gShaders)[1].Use();
				(*gShaders)[1].SetVec3("spotLight.position", gCamera->position);
				(*gShaders)[1].SetVec3("spotLight.direction", gCamera->direction);

				//Update the view and projection matrices in the shader
				(*gShaders)[1].SetMat4("view", view);
				(*gShaders)[1].SetMat4("projection", projection);

				//Disable writing to the stencil buffer
				glStencilMask(0x00);

				//Render the plane
				gPlaneModel->Draw((*gShaders)[1]);

				//Setup the stencil buffer operations for writing
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);

				//Render the cubes
				for (int i = 0; i < 2; ++i)
				{
					gCubeModels[i].Draw((*gShaders)[1]);
				}

				if (outlineEffectEnabled)
				{
					//Setup the stencil buffer operations for the outline effect
					glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
					//Disable writing to the stencil buffer
					glStencilMask(0x00);

					//Disable the depth buffer operations
					glDisable(GL_DEPTH_TEST);

					//Use the border shader
					(*gShaders)[4].Use();

					//Update the view and projection matrices in the shader
					(*gShaders)[4].SetMat4("view", view);
					(*gShaders)[4].SetMat4("projection", projection);

					//Render the scaled up cubes for the outline effect
					for (int i = 0; i < 2; ++i)
					{
						//Save the original scale
						glm::vec3 originalScale = gCubeModels[i].getScale();

						//Set the new scale
						gCubeModels[i].setScale(originalScale * 1.1f);

						//Draw the cube
						gCubeModels[i].Draw((*gShaders)[4]);

						//Restore the original scale
						gCubeModels[i].setScale(originalScale);
					}

					//Re-enable depth buffer writing
					glEnable(GL_DEPTH_TEST);

					//Reset the stencil buffer settings
					glStencilMask(0xFF);
					glStencilFunc(GL_ALWAYS, 1, 0xFF);
				}

				//Create a sorted map of the glass planes based on distance from the camera
				std::map<float, PlaneModel*> sorted;
				for (GLuint i = 0; i < 5; ++i)
				{
					float distance = glm::length(gCamera->position - gGlassPlaneModels[i].getPosition());
					sorted[distance] = &gGlassPlaneModels[i];
				}

				//Render the glass planes in back-to-front order
				for(auto it = sorted.rbegin(); it != sorted.rend(); ++it)
				{
					it->second->Draw((*gShaders)[1]);
				}

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
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

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
	glDepthFunc(GL_LESS);

	//Enable stencil testing
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	//Define the directional light properties
	gDirectionalLight.position = glm::vec3(0.0f, 0.0f, 0.0f);
	gDirectionalLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	gDirectionalLight.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	gDirectionalLight.specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//Define the spotlight properties
	gSpotLight.position = glm::vec3(0.0f, 0.0f, 0.0f);
	gSpotLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	gSpotLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	gSpotLight.specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//Define the point lights properties
	gPointLights[0].position = glm::vec3(10.0f, 10.5f, 10.0f);
	gPointLights[0].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	gPointLights[0].diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	gPointLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	KJK_INFO("Initialized OpenGL!");

	//Return the success flag
	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Create the shaders
	gShaders.emplace(std::array<Shader, 5>
	{
		Shader("assets/shaders/shader.vert", "assets/shaders/shader2.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/shader2.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/lightSourceShader.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/DistanceShader.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/BorderShader.frag")
	});

	//Load two cube models
	gCubeModels = new CubeModel[2]
	{
		CubeModel("assets/marble.jpg", "assets/marble.jpg"),
		CubeModel("assets/marble.jpg", "assets/marble.jpg")
	};
	//Set the positions of the cube models
	gCubeModels[0].setPosition(glm::vec3(-1.5f, 0.0001f, -1.0f));
	gCubeModels[1].setPosition(glm::vec3(1.5f, 0.0001f, 0.0f));

	//Load the plane model
	gPlaneModel = new PlaneModel("assets/metal.png", "assets/metal.png");
	//Set the position of the plane model
	gPlaneModel->setPosition(glm::vec3(0.0f, -0.5f, 0.0f));
	//Set the scale of the plane model
	gPlaneModel->setScale(glm::vec3(6.0f, 1.0f, 6.0f));
	//Set the texture scale of the plane model
	gPlaneModel->setTextureScale(2.0f);

	//Load the grass models
	gGlassPlaneModels = new PlaneModel[5]
	{
		PlaneModel("assets/blending_transparent_window.png", "assets/blending_transparent_window.png"),
		PlaneModel("assets/blending_transparent_window.png", "assets/blending_transparent_window.png"),
		PlaneModel("assets/blending_transparent_window.png", "assets/blending_transparent_window.png"),
		PlaneModel("assets/blending_transparent_window.png", "assets/blending_transparent_window.png"),
		PlaneModel("assets/blending_transparent_window.png", "assets/blending_transparent_window.png")
	};
	//Set the positions of the grass models
	gGlassPlaneModels[0].setPosition(glm::vec3(-1.5f, -0.0001f, -0.48f));
	gGlassPlaneModels[1].setPosition(glm::vec3( 1.5f, -0.0001f,  0.51f));
	gGlassPlaneModels[2].setPosition(glm::vec3( 0.0f, -0.0001f,  0.7f));
	gGlassPlaneModels[3].setPosition(glm::vec3(-0.3f, -0.0001f, -2.3f));
	gGlassPlaneModels[4].setPosition(glm::vec3( 0.5f, -0.0001f, -0.6f));
	//Set the rotations of the grass models
	for(int i = 0; i < 5; ++i)
	{
		//Rotate the grass models so they stand upright
		gGlassPlaneModels[i].setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	}

	//Use the first shader program
	(*gShaders)[1].Use();

	//Set color uniforms for the spotlight
	(*gShaders)[1].SetVec4("spotLight.ambient", glm::vec4(0.0f));
	(*gShaders)[1].SetVec4("spotLight.diffuse", glm::vec4(0.0f));
	(*gShaders)[1].SetVec4("spotLight.specular", glm::vec4(0.0f));
	//Set spotlight cutoff angles
	(*gShaders)[1].SetFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	(*gShaders)[1].SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));
	//Set spotlight attenuation factors
	(*gShaders)[1].SetFloat("spotLight.constant", 1.0f);
	(*gShaders)[1].SetFloat("spotLight.linear", 0.045f);
	(*gShaders)[1].SetFloat("spotLight.quadratic", 0.0075f);

	//Set color uniforms for the directional light
	(*gShaders)[1].SetVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
	(*gShaders)[1].SetVec4("dirLight.ambient", gDirectionalLight.ambient);
	(*gShaders)[1].SetVec4("dirLight.diffuse", gDirectionalLight.diffuse);
	(*gShaders)[1].SetVec4("dirLight.specular", gDirectionalLight.specular);

	//Set color uniforms for the point light
	(*gShaders)[1].SetVec3("pointLights[0].position", gPointLights[0].position);
	(*gShaders)[1].SetVec4("pointLights[0].ambient", gPointLights[0].ambient);
	(*gShaders)[1].SetVec4("pointLights[0].diffuse", gPointLights[0].diffuse);
	(*gShaders)[1].SetVec4("pointLights[0].specular", gPointLights[0].specular);
	//Set point light attenuation factors
	(*gShaders)[1].SetFloat("pointLights[0].constant", 1.0f);
	(*gShaders)[1].SetFloat("pointLights[0].linear", 0.09f);
	(*gShaders)[1].SetFloat("pointLights[0].quadratic", 0.032f);

	//Set material shininess
	(*gShaders)[1].SetFloat("material.shininess", 32.0f);

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

	//Delete the cube models
	delete[] gCubeModels;

	//Delete the plane model
	delete gPlaneModel;

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