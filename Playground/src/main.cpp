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
//Change the shader program
void changeShader(GLint shaderIndex);
//Recreate framebuffer objects on window resize
void recreateFramebuffers();

//Render example scene
void renderExampleScene(float timeValue, bool showNormals, bool outlineEffectEnabled, glm::mat4 view, glm::mat4 projection, int shadowType = 0);
//Render space scene
void renderSpaceScene(float timeValue, glm::mat4 view, glm::mat4 projection, int shadowType = 0);

//Global variables
int SCREEN_WIDTH{ 800 };
int SCREEN_HEIGHT{ 600 };

//The window to render to
SDL_Window* gWindow = nullptr;

//Context for OpenGL
SDL_GLContext gContext;

//Framebuffer object ID
GLuint gFBO{ 0 };
//Texture that servers as the framebuffer color attachment
GLuint gFBOTexture{ 0 };
//Renderbuffer object for depth and stencil attachments
GLuint gRBO{ 0 };
//Screen quad VAO and VBO
GLuint gScreenQuadVAO{ 0 };
GLuint gScreenQuadVBO{ 0 };

//Multisample framebuffer object ID
GLuint gMultisampleFBO{ 0 };
//Multisample texture that serves as the framebuffer color attachment
GLuint gMultisampleFBOTexture{ 0 };
//Multisample renderbuffer object for depth and stencil attachments
GLuint gMultisampleRBO{ 0 };

//Shadow map framebuffer object ID
GLuint gShadowMapFBO{ 0 };
//Shadow map texture ID
GLuint gShadowMapTexture{ 0 };
//Shadow map dimensions
const GLuint SHADOW_WIDTH{ 4096 };
const GLuint SHADOW_HEIGHT{ 4096 };
glm::mat4 gLightSpaceMatrix;

//Shadow map framebuffer object ID for a point light
GLuint gPointLightShadowMapFBO{ 0 };
//Shadow map cube map texture ID for a point light
GLuint gPointLightShadowMapCubeTexture{ 0 };
//Shadow map far plane for point light
float gPointLightShadowFarPlane{ 25.0f };

//Shader program IDs
std::optional<std::array<Shader, 20>> gShaders;
//Current shader index
GLint gCurrentShaderIndex{ 0 };

//Uniform buffer object ID for matrices
GLuint gMatricesUBO{ 0 };

//Camera object
Camera* gCamera;

//Skybox cube
CubeModel* gSkyboxCube;

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

//Detailed model object
Model* gModel;

//Planet model object
Model* gPlanetModel;
//Asteroid model object
Model* gAsteroidModel;
//Asteroid instances model matrices
glm::mat4* gAsteroidModelMatrices;
//Define the amount of asteroids to instantiate
GLuint gAsteroidInstanceAmount = 10000;
//Asteroid instance VBO
GLuint gAsteroidInstanceVBO{ 0 };

//Cube model objects
CubeModel* gCubeModels;

//Plane model object
PlaneModel* gPlaneModel;

//Glass objects
PlaneModel* gGlassPlaneModels;

//Reflective cube object
CubeModel* gReflectiveCubeModel;
//Refractive cube object
CubeModel* gRefractiveCubeModel;

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

			//Highlight outline effect settings
			bool outlineEffectEnabled = false;

			//Postprocessing effect settings
			bool applyPostProcessing = false;

			//Show normal vectors setting
			bool showNormals = false;

			//SHow depth map setting
			bool showDepthMap = false;

			//Current selected scene setting
			int currentScene = 1;

			//Initialize time value
			float timeValue = 0.0f;

			//Enum for input state control
			enum class InputState
			{
				NONE,
				MIX,
				FOV,
				SCENE
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

						//Recreate framebuffer objects
						recreateFramebuffers();
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
						case SDLK_3: //Enable scene change control
							inputState = InputState::SCENE;
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

							for (int i : {1, 8, 10})
							{
								changeShader(i);
								if (flashlightEnabled)
								{
									(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.ambient", gSpotLight.ambient);
									(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.diffuse", gSpotLight.diffuse);
									(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.specular", gSpotLight.specular);
								}
								else
								{
									(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.ambient", glm::vec4(0.0f));
									(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.diffuse", glm::vec4(0.0f));
									(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.specular", glm::vec4(0.0f));
								}
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
						case SDLK_T: //Toggle postprocessing effect
							applyPostProcessing = !applyPostProcessing;
							break;
						case SDLK_N: //Toggle normal vector display
							showNormals = !showNormals;
							break;
						case SDLK_0:
							showDepthMap = !showDepthMap;
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
								changeShader(1);
								(*gShaders)[gCurrentShaderIndex].SetFloat("mixValue", mixValue);
								break;
							case InputState::FOV: //Increase the FoV
								gCamera->fov += 5.0f;
								if (gCamera->fov > 360.0f)
								{
									gCamera->fov = 360.0f;
								}
								break;
							case InputState::SCENE: //Change scene up
								currentScene++;
								if (currentScene > 1)
									currentScene = 1;
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
								changeShader(1);
								(*gShaders)[gCurrentShaderIndex].SetFloat("mixValue", mixValue);
								break;
							case InputState::FOV:
								gCamera->fov -= 5.0f;
								if (gCamera->fov < 0.0f)
								{
									gCamera->fov = 0.0f;
								}
								break;
							case InputState::SCENE: //Change scene down
								currentScene--;
								if (currentScene < 0)
									currentScene = 0;
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

				//Resize the viewport to the shadow map size
				glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
				//Bind the shadow map framebuffer
				glBindFramebuffer(GL_FRAMEBUFFER, gShadowMapFBO);

				//Enable depth testing
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				glDepthMask(GL_TRUE);

				//Clear the depth buffer
				glClear(GL_DEPTH_BUFFER_BIT);

				//Declare the variable for orthogonal sizing
				float orthogonalSize = 0;
				//Render the scene to the shadow map
				switch (currentScene)
				{
				case 0:
					orthogonalSize = 80.0f;
					break;
				case 1:
					orthogonalSize = 10.0f;
					break;
				default:
					orthogonalSize = 80.0f;
					break;
				}

				//Define directional light projection matrix
				glm::mat4 lightProjection = glm::ortho(-orthogonalSize, orthogonalSize, -orthogonalSize, orthogonalSize, 0.1f, 200.0f);
				//Define directional light view matrix
				glm::mat4 lightView = glm::lookAt(gDirectionalLight.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

				//Change to a simple depth shader
				changeShader(11);

				//Set the light space matrix uniform
				gLightSpaceMatrix = lightProjection * lightView;
				(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);

				//Update the view and projection matrices in the UBO
				glBindBuffer(GL_UNIFORM_BUFFER, gMatricesUBO);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(lightProjection));
				glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(lightView));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				//Render the scene to the shadow map
				switch (currentScene)
				{
				case 0:
					renderSpaceScene(timeValue, lightView, lightProjection, 1);
					break;
				case 1:
					//Enable front face culling to reduce shadow acne
					glEnable(GL_CULL_FACE);
					glCullFace(GL_FRONT);

					renderExampleScene(timeValue, showNormals, outlineEffectEnabled, lightView, lightProjection, 1);
					break;
				default:
					renderExampleScene(timeValue, showNormals, outlineEffectEnabled, lightView, lightProjection, 1);
					break;
				}

				//Bind the point light shadow map framebuffer
				glBindFramebuffer(GL_FRAMEBUFFER, gPointLightShadowMapFBO);

				//Disable face culling
				glDisable(GL_CULL_FACE);

				//Clear the depth buffer
				glClear(GL_DEPTH_BUFFER_BIT);

				//Define a point light projection matrix
				glm::mat4 pointLightProjection = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 0.1f, gPointLightShadowFarPlane);
				//Define the point light view matrices for each cube map face
				std::vector<glm::mat4> pointLightViews;
				pointLightViews.push_back(glm::lookAt(gPointLights[0].position, gPointLights[0].position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
				pointLightViews.push_back(glm::lookAt(gPointLights[0].position, gPointLights[0].position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
				pointLightViews.push_back(glm::lookAt(gPointLights[0].position, gPointLights[0].position + glm::vec3 (0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
				pointLightViews.push_back(glm::lookAt(gPointLights[0].position, gPointLights[0].position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
				pointLightViews.push_back(glm::lookAt(gPointLights[0].position, gPointLights[0].position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
				pointLightViews.push_back(glm::lookAt(gPointLights[0].position, gPointLights[0].position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

				//Update the projection matrix in the UBO
				glBindBuffer(GL_UNIFORM_BUFFER, gMatricesUBO);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pointLightProjection));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				//Calculate and store the point light projection-view matrices
				std::vector<glm::mat4> pointLightProjectionViews;
				for (GLuint i = 0; i < 6; i++)
				{
					pointLightProjectionViews.push_back(pointLightProjection * pointLightViews[i]);
				}

				//Set uniforms for all point light shadow shaders
				for(int i : {16, 17, 18, 19})
				{
					//Set the point light projection matrix uniform
					changeShader(i);

					//Set the far plane and light position uniforms
					(*gShaders)[gCurrentShaderIndex].SetFloat("far_plane", gPointLightShadowFarPlane);
					(*gShaders)[gCurrentShaderIndex].SetVec3("lightPos", gPointLights[0].position);

					//Set the point light projection-view matrices uniforms
					for (GLuint j = 0; j < 6; j++)
					{
						std::string uniformName = "shadowMatrices[" + std::to_string(j) + "]";
						(*gShaders)[gCurrentShaderIndex].SetMat4(uniformName, pointLightProjectionViews[j]);
					}

					//For the exploding point shader, set the views uniform
					if (i == 18)
					{
						for (GLuint j = 0; j < 6; j++)
						{
							std::string uniformName = "views[" + std::to_string(j) + "]";
							(*gShaders)[gCurrentShaderIndex].SetMat4(uniformName, pointLightViews[j]);
						}
					}
				}

				//Change to a simple point light depth shader
				changeShader(16);

				//Render the scene to the point light shadow map
				switch (currentScene)
				{
					case 0:
						renderSpaceScene(timeValue, pointLightViews[0], pointLightProjection, 2);
						break;
					case 1:
						renderExampleScene(timeValue, showNormals, outlineEffectEnabled, pointLightViews[0], pointLightProjection, 2);
						break;
					default:
						renderExampleScene(timeValue, showNormals, outlineEffectEnabled, pointLightViews[0], pointLightProjection, 2);
						break;
				}

				//Enable back face culling
				glCullFace(GL_BACK);

				//Change the viewport to the screen size
				glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				//Use the created framebuffer
				glBindFramebuffer(GL_FRAMEBUFFER, gMultisampleFBO);

				//Set the clear color
				glClearColor(0.05f, 0.0f, 0.05f, 1.0f);
				//Clear the buffers
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				//Enable depth testing
				glEnable(GL_DEPTH_TEST);

				//Set to polygon wireframe mode
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				//Calculate the time value
				if (enableMovement)
				{
					timeValue += deltaTime;
				}
				
				//Define a view matrix
				glm::mat4 view = gCamera->GetViewMatrix();
				
				//Define a projection matrix
				glm::mat4 projection = glm::mat4(1.0f);
				projection = glm::perspective(glm::radians(gCamera->fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

				//Update the view and projection matrices in the UBO
				glBindBuffer(GL_UNIFORM_BUFFER, gMatricesUBO);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
				glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				//Check scene number and render the correct scene
				switch (currentScene)
				{
				case 0:
					renderSpaceScene(timeValue, view, projection);
					break;
				case 1:
					renderExampleScene(timeValue, showNormals, outlineEffectEnabled, view, projection);
					break;
				default:
					renderExampleScene(timeValue, showNormals, outlineEffectEnabled, view, projection);
					break;
				}

				//Blit the multisample framebuffer to the normal framebuffer
				glBindFramebuffer(GL_READ_FRAMEBUFFER, gMultisampleFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gFBO);
				glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

				//Bind the default framebuffer to render to the screen
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				//Clear the screen
				glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				//Choose the shader for rendering the screen quad
				if(showDepthMap)
				{
					//Use the depth map shader
					changeShader(12);
				}
				else if (applyPostProcessing)
				{
					//Use the post processing shader
					changeShader(0);
				}
				else
				{
					//Use the simple texture shader
					changeShader(2);
				}

				//Bind the screen quad VAO
				glBindVertexArray(gScreenQuadVAO);

				//Disable depth testing
				glDisable(GL_DEPTH_TEST);

				//Bind the appropriate texture for the screen quad
				glActiveTexture(GL_TEXTURE0);
				if(showDepthMap)
				{
					//Bind the shadow map texture
					glBindTexture(GL_TEXTURE_2D, gShadowMapTexture);
				}
				else
				{
					//Bind the framebuffer texture
					glBindTexture(GL_TEXTURE_2D, gFBOTexture);
				}

				//Draw the screen quad
				glDrawArrays(GL_TRIANGLES, 0, 6);

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
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 4);

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

	//Enable face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//Enable multisampling
	glEnable(GL_MULTISAMPLE);

	//Generate a framebuffer object
	glGenFramebuffers(1, &gFBO);
	//Bind the framebuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);

	//Create a texture to serve as the framebuffer color attachment
	glGenTextures(1, &gFBOTexture);
	glBindTexture(GL_TEXTURE_2D, gFBOTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);
	//Attach the texture to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gFBOTexture, 0);

	//Create a renderbuffer object for depth and stencil attachments
	glGenRenderbuffers(1, &gRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	//Unbind the renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//Attach the renderbuffer to the framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gRBO);

	//Check if the framebuffer's status is complete
	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		KJK_ERROR("Post-processing framebuffer is not complete: {0}!", framebufferStatus);
		success = false;
	}

	//Generate a multisample framebuffer object
	glGenFramebuffers(1, &gMultisampleFBO);
	//Bind the multisample framebuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, gMultisampleFBO);

	//Create a multisample texture to serve as the framebuffer color attachment
	glGenTextures(1, &gMultisampleFBOTexture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gMultisampleFBOTexture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TRUE);
	glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Unbind the multisample texture
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	//Attach the texture to the multisample framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, gMultisampleFBOTexture, 0);

	//Create a multisample renderbuffer object for depth and stencil attachments
	glGenRenderbuffers(1, &gMultisampleRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, gMultisampleRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	//Unbind the multisample renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//Attach the renderbuffer to the multisample framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gMultisampleRBO);

	//Check if the multisample framebuffer's status is complete
	framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		KJK_ERROR("Multisample framebuffer is not complete: {0}!", framebufferStatus);
		success = false;
	}

	//Generate the shadow map framebuffer
	glGenFramebuffers(1, &gShadowMapFBO);
	//Bind the shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gShadowMapFBO);

	//Create the shadow map texture
	glGenTextures(1, &gShadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, gShadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//Attach the texture to the shadow map framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gShadowMapTexture, 0);
	//Disable color buffer writes
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//Check if the shadow map framebuffer is complete
	framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		KJK_ERROR("Shadow map framebuffer is not complete: {0}!", framebufferStatus);
		success = false;
	}

	//Create the point light shadow map framebuffer
	glGenFramebuffers(1, &gPointLightShadowMapFBO);
	//Bind the point light shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gPointLightShadowMapFBO);

	//Create the point light shadow map cube map texture
	glGenTextures(1, &gPointLightShadowMapCubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gPointLightShadowMapCubeTexture);
	for (GLuint i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Attach the cube map texture to the point light shadow map framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gPointLightShadowMapCubeTexture, 0);
	//Disable color buffer writes
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//Check if the point light shadow map framebuffer is complete
	framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		KJK_ERROR("Point light shadow map framebuffer is not complete: {0}!", framebufferStatus);
		success = false;
	}

	//Unbind the shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Generate the UBO for matrices
	glGenBuffers(1, &gMatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, gMatricesUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//Bind the UBO to binding point 0
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, gMatricesUBO, 0, 2 * sizeof(glm::mat4));

	//Setup the screen quad VAO and VBO
	glGenVertexArrays(1, &gScreenQuadVAO);
	glGenBuffers(1, &gScreenQuadVBO);
	glBindVertexArray(gScreenQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gScreenQuadVBO);
	float screenQuadVertices[] = {
		//Positions   //TexCoords
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), &screenQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	//Define the directional light properties
	gDirectionalLight.position = glm::vec3(-20.0f, 40.0f, -10.0f);
	gDirectionalLight.ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	gDirectionalLight.diffuse = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	gDirectionalLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	//Define the spotlight properties
	gSpotLight.position = glm::vec3(0.0f, 0.0f, 0.0f);
	gSpotLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	gSpotLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	gSpotLight.specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//Define the point lights properties
	gPointLights[0].position = glm::vec3(1.0f, 0.00001f, 1.0f);
	gPointLights[0].ambient = glm::vec4(0.05f, 0.0f, 0.0f, 1.0f);
	gPointLights[0].diffuse = glm::vec4(0.8f, 0.0f, 0.0f, 1.0f);
	gPointLights[0].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	KJK_INFO("Initialized OpenGL!");

	//Return the success flag
	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Create the shaders
	gShaders.emplace(std::array<Shader, 20>
	{
		Shader("assets/shaders/FrameBufferShader.vert", "assets/shaders/FrameBufferShader.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/shader2.frag"),
		Shader("assets/shaders/FrameBufferShader.vert", "assets/shaders/BasicShader.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/DistanceShader.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/BorderShader.frag"),
		Shader("assets/shaders/Cubemap.vert", "assets/shaders/Cubemap.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/ReflectiveShader.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/RefractiveShader.frag"),
		Shader("assets/shaders/shaderExplode.vert", "assets/shaders/explode.geom", "assets/shaders/shader2.frag"),
		Shader("assets/shaders/NormalVectorShader.vert", "assets/shaders/NormalVectorShader.geom", "assets/shaders/NormalVectorShader.frag"),
		Shader("assets/shaders/InstanceShader.vert", "assets/shaders/InstanceShader.frag"),
		Shader("assets/shaders/simpleDepthShader.vert", "assets/shaders/simpleDepthShader.frag"),
		Shader("assets/shaders/FrameBufferShader.vert", "assets/shaders/DepthMapShader.frag"),
		Shader("assets/shaders/instancedDepthShader.vert", "assets/shaders/simpleDepthShader.frag"),
		Shader("assets/shaders/simpleDepthShader.vert", "assets/shaders/transparentDepthShader.frag"),
		Shader("assets/shaders/explodingDepthShader.vert", "assets/shaders/explodingDepthShader.geom", "assets/shaders/simpleDepthShader.frag"),
		Shader("assets/shaders/simplePointDepthShader.vert", "assets/shaders/simplePointDepthShader.geom", "assets/shaders/simplePointDepthShader.frag"),
		Shader("assets/shaders/simplePointDepthShader.vert", "assets/shaders/simplePointDepthShader.geom", "assets/shaders/transparentPointDepthShader.frag"),
		Shader("assets/shaders/explodingPointDepthShader.vert", "assets/shaders/explodingPointDepthShader.geom", "assets/shaders/simplePointDepthShader.frag"),
		Shader("assets/shaders/instancedPointDepthShader.vert", "assets/shaders/simplePointDepthShader.geom", "assets/shaders/simpleDepthShader.frag"),
	});

	//Load two cube models
	gCubeModels = new CubeModel[2]
	{
		CubeModel("assets/container.jpg", "assets/container.jpg"),
		CubeModel("assets/container.jpg", "assets/container.jpg")
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
	gGlassPlaneModels[0].setPosition(glm::vec3(-1.5f, 0.0001f, -0.48f));
	gGlassPlaneModels[1].setPosition(glm::vec3( 1.5f, 0.0001f,  0.51f));
	gGlassPlaneModels[2].setPosition(glm::vec3( 0.0f, 0.0001f,  0.7f));
	gGlassPlaneModels[3].setPosition(glm::vec3(-0.3f, 0.0001f, -2.3f));
	gGlassPlaneModels[4].setPosition(glm::vec3( 0.5f, 0.0001f, -0.6f));
	//Set the rotations of the grass models
	for(int i = 0; i < 5; ++i)
	{
		//Rotate the grass models so they stand upright
		gGlassPlaneModels[i].setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	}

	//Load the skybox cube
	gSkyboxCube = new CubeModel(nullptr, nullptr, false,
		std::vector<std::string>
		{
			"assets/skybox/right.jpg",
			"assets/skybox/left.jpg",
			"assets/skybox/top.jpg",
			"assets/skybox/bottom.jpg",
			"assets/skybox/front.jpg",
			"assets/skybox/back.jpg"
		}
	);
	//Set the scale of the skybox cube
	gSkyboxCube->setScale(glm::vec3(2.0f, 2.0f, 2.0f));

	//Load the reflective cube model
	gReflectiveCubeModel = new CubeModel(nullptr, nullptr, false,
		std::vector<std::string>
		{
			"assets/skybox/right.jpg",
			"assets/skybox/left.jpg",
			"assets/skybox/top.jpg",
			"assets/skybox/bottom.jpg",
			"assets/skybox/front.jpg",
			"assets/skybox/back.jpg"
		}
	);
	//Set the position of the reflective cube model
	gReflectiveCubeModel->setPosition(glm::vec3(-0.5f, 5.0f, -2.0f));

	//Load the refractive cube model
	gRefractiveCubeModel = new CubeModel(nullptr, nullptr, false,
		std::vector<std::string>
		{
			"assets/skybox/right.jpg",
			"assets/skybox/left.jpg",
			"assets/skybox/top.jpg",
			"assets/skybox/bottom.jpg",
			"assets/skybox/front.jpg",
			"assets/skybox/back.jpg"
		}
	);
	//Set the position of the refractive cube model
	gRefractiveCubeModel->setPosition(glm::vec3(4.0f, 5.0f, 0.0f));

	//Load the detailed model
	gModel = new Model("assets/backpack/backpack.obj");

	//Load the planet model
	gPlanetModel = new Model("assets/planet/planet.obj");
	//Load the asteroid model
	gAsteroidModel = new Model("assets/rock/rock.obj");

	//Create the array to store their positions
	gAsteroidModelMatrices = new glm::mat4[gAsteroidInstanceAmount];
	//Initialize a random seed
	srand(SDL_GetTicks());
	//Declare the model variables
	float radius = 60.0f;
	float offset = 10.0f;
	
	//Iterate over each instance to set their unique translations, scale and rotations
	for (GLuint i = 0; i < gAsteroidInstanceAmount; i++)
	{
		//Initialize the model matrix
		glm::mat4 model = glm::mat4(1.0f);

		//Calculate random displacement along a circle
		float angle = (float)i / (float)gAsteroidInstanceAmount * 360.f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		//Apply translation
		model = glm::translate(model, glm::vec3(x, y, z));

		//Calculate a random scale
		float scale = (rand() % 20) / 100.0f + 0.05f;
		//Apply the scale
		model = glm::scale(model, glm::vec3(scale));

		//Calculate a random rotation
		float rotX = (rand() % 360);
		float rotY = (rand() % 360);
		float rotZ = (rand() % 360);
		//Apply the rotation
		model = glm::rotate(model, rotX, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));

		//Add to the list of matrices
		gAsteroidModelMatrices[i] = model;
	}

	//Generate a vertex buffer object for the asteroid instance matrices
	glGenBuffers(1, &gAsteroidInstanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gAsteroidInstanceVBO);
	//Fill the buffer with the model matrices
	glBufferData(GL_ARRAY_BUFFER, gAsteroidInstanceAmount * sizeof(glm::mat4), &gAsteroidModelMatrices[0], GL_STATIC_DRAW);

	//Configure the vertex attributes for each asteroid mesh
	for (GLuint i = 0; i < gAsteroidModel->GetMeshes().size(); i++)
	{
		//Get a vertex array object id for the model mesh
		GLuint vao = gAsteroidModel->GetMeshes()[i].GetVAO();
		//Bind the vertex array
		glBindVertexArray(vao);

		//Update the vertex attributes with the information about the model matrices
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		//Tell OpenGl when to update the context of the vertex attribute
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		//Unbind the vertex array
		glBindVertexArray(0);
	}


	//Iterate over the 1st and 8th shader programs
	for (GLint i : {1, 8, 10})
	{
		//Use a shader program
		changeShader(i);

		//Set the texture scale uniform
		(*gShaders)[gCurrentShaderIndex].SetFloat("textureScale", 1.0f);

		//Set color uniforms for the spotlight
		(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.ambient", glm::vec4(0.0f));
		(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.diffuse", glm::vec4(0.0f));
		(*gShaders)[gCurrentShaderIndex].SetVec4("spotLight.specular", glm::vec4(0.0f));
		//Set spotlight cutoff angles
		(*gShaders)[gCurrentShaderIndex].SetFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		(*gShaders)[gCurrentShaderIndex].SetFloat("spotLight.outerCutOff", glm::cos(glm::radians(20.0f)));
		//Set spotlight attenuation factors
		(*gShaders)[gCurrentShaderIndex].SetFloat("spotLight.constant", 1.0f);
		(*gShaders)[gCurrentShaderIndex].SetFloat("spotLight.linear", 0.07f);
		(*gShaders)[gCurrentShaderIndex].SetFloat("spotLight.quadratic", 0.017f);

		//Set color uniforms for the directional light
		(*gShaders)[gCurrentShaderIndex].SetVec3("dirLight.direction", glm::vec3(0.0f) - gDirectionalLight.position);
		(*gShaders)[gCurrentShaderIndex].SetVec4("dirLight.ambient", gDirectionalLight.ambient);
		(*gShaders)[gCurrentShaderIndex].SetVec4("dirLight.diffuse", gDirectionalLight.diffuse);
		(*gShaders)[gCurrentShaderIndex].SetVec4("dirLight.specular", gDirectionalLight.specular);

		//Set color uniforms for the point light
		(*gShaders)[gCurrentShaderIndex].SetVec3("pointLights[0].position", gPointLights[0].position);
		(*gShaders)[gCurrentShaderIndex].SetVec4("pointLights[0].ambient", gPointLights[0].ambient);
		(*gShaders)[gCurrentShaderIndex].SetVec4("pointLights[0].diffuse", gPointLights[0].diffuse);
		(*gShaders)[gCurrentShaderIndex].SetVec4("pointLights[0].specular", gPointLights[0].specular);
		//Set point light attenuation factors
		(*gShaders)[gCurrentShaderIndex].SetFloat("pointLights[0].constant", 1.0f);
		(*gShaders)[gCurrentShaderIndex].SetFloat("pointLights[0].linear", 0.14f);
		(*gShaders)[gCurrentShaderIndex].SetFloat("pointLights[0].quadratic", 0.07f);

		//Set point light world position uniform
		(*gShaders)[gCurrentShaderIndex].SetVec3("pointLightsWorld[0].position", gPointLights[0].position);

		//Set material shininess
		(*gShaders)[gCurrentShaderIndex].SetFloat("material.shininess", 32.0f);

		//Set the far plane distance for point light shadow mapping
		(*gShaders)[gCurrentShaderIndex].SetFloat("far_plane", gPointLightShadowFarPlane);
	}

	//Change to the framebuffer shader and set the texture uniform
	changeShader(0);
	(*gShaders)[gCurrentShaderIndex].SetInt("screenTexture", 0);

	//Change to the depth map shader and set the depth map texture uniform
	changeShader(12);
	(*gShaders)[gCurrentShaderIndex].SetInt("depthMap", 0);

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

	//Delete the space scene objects
	delete gPlanetModel;
	delete gAsteroidModel;
	delete[] gAsteroidModelMatrices;

	//Delete the cube models
	delete[] gCubeModels;

	//Delete the plane model
	delete gPlaneModel;

	//Delete the glass models
	delete[] gGlassPlaneModels;

	//Delete the skybox cube
	delete gSkyboxCube;

	//Delete the reflective cube model
	delete gReflectiveCubeModel;
	//Delete the refractive cube model
	delete gRefractiveCubeModel;

	//Delete the screen quad VAO and VBO
	glDeleteVertexArrays(1, &gScreenQuadVAO);
	glDeleteBuffers(1, &gScreenQuadVBO);

	//Delete the renderbuffer objects
	glDeleteRenderbuffers(1, &gRBO);
	glDeleteRenderbuffers(1, &gMultisampleRBO);

	//Delete the textures used for the framebuffers
	glDeleteTextures(1, &gFBOTexture);
	glDeleteTextures(1, &gMultisampleFBOTexture);

	//Delete the framebuffer objects
	glDeleteFramebuffers(1, &gFBO);
	glDeleteFramebuffers(1, &gMultisampleFBO);

	//Delete the asteroid instance VBO
	glDeleteBuffers(1, &gAsteroidInstanceVBO);

	//Delete the shadow map texture and framebuffer
	glDeleteTextures(1, &gShadowMapTexture);
	glDeleteFramebuffers(1, &gShadowMapFBO);

	//Delete the cube shadow map texture and framebuffer
	glDeleteTextures(1, &gPointLightShadowMapCubeTexture);
	glDeleteFramebuffers(1, &gPointLightShadowMapFBO);

	//Delete the UBO for matrices
	glDeleteBuffers(1, &gMatricesUBO);

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

//Change the shader program
void changeShader(GLint index)
{
	if(gShaders.has_value() && index >= 0 && index < static_cast<GLint>(gShaders->size()))
	{
		gCurrentShaderIndex = index;
		(*gShaders)[gCurrentShaderIndex].Use();
	}
}

void recreateFramebuffers()
{
	//Recreate the framebuffer texture
	glBindTexture(GL_TEXTURE_2D, gFBOTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Recreate the renderbuffer storage
	glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//Recreate the multisample framebuffer texture
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gMultisampleFBOTexture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	//Recreate the multisample renderbuffer storage
	glBindRenderbuffer(GL_RENDERBUFFER, gMultisampleRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//Unbind any framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//Render an example scene that showcases many OpenGL techniques.
void renderExampleScene(float timeValue, bool showNormals, bool outlineEffectEnabled, glm::mat4 view, glm::mat4 projection, int shadowType)
{
	if(shadowType == 0)
	{
		//Enable the shader for the model
		changeShader(1);
		
		//Bind the shadow map texture
		glActiveTexture(GL_TEXTURE25);
		glBindTexture(GL_TEXTURE_2D, gShadowMapTexture);
		//Load the shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMap", 25);
		//Load the light space matrix uniform
		(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);

		//Bind the point light shadow map cube texture
		glActiveTexture(GL_TEXTURE26);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gPointLightShadowMapCubeTexture);
		//Load the point light shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMapPoint", 26);

		//Set the far plane distance uniform
		(*gShaders)[gCurrentShaderIndex].SetFloat("far_plane", gPointLightShadowFarPlane);

		//Update the spotlight position and direction uniforms
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.position", gCamera->position);
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.direction", gCamera->direction);

		//Disable writing to the stencil buffer
		glStencilMask(0x00);
	}

	//Render the plane
	gPlaneModel->Draw((*gShaders)[gCurrentShaderIndex]);

	if (shadowType == 0)
	{
		//Setup the stencil buffer operations for writing
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}

	//Render the cubes
	for (int i = 0; i < 2; ++i)
	{
		gCubeModels[i].Draw((*gShaders)[gCurrentShaderIndex]);
	}

	//Disable face culling
	glDisable(GL_CULL_FACE);

	if (shadowType == 0)
	{
		//Disable writing to the stencil buffer
		glStencilMask(0x00);

		//Change the shader to use the explosion geometry effect
		changeShader(8);

		//Bind the shadow map texture
		glActiveTexture(GL_TEXTURE25);
		glBindTexture(GL_TEXTURE_2D, gShadowMapTexture);
		//Load the shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMap", 25);
		//Load the light space matrix uniform
		(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);

		//Bind the point light shadow map cube texture
		glActiveTexture(GL_TEXTURE26);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gPointLightShadowMapCubeTexture);
		//Load the point light shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMapPoint", 26);

		//Update the spotlight position and direction uniforms
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.position", gCamera->position);
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.direction", gCamera->direction);
	}
	else if (shadowType == 1)
	{
		//Switch to the exploding depth shader
		changeShader(15);

		//Set the light space matrix uniform
		(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);
	}
	else if (shadowType == 2)
	{
		//Switch to the exploding point depth shader
		changeShader(18);
	}

	//Set the time uniform
	(*gShaders)[gCurrentShaderIndex].SetFloat("time", timeValue / 4.0f);

	//Set the model matrix for the detailed model
	glm::mat4 model = glm::mat4(1.0f);
	//Move the model back a bit
	model = glm::translate(model, glm::vec3(2.0f, -0.15f, -2.5f));
	//Scale the model down
	model = glm::scale(model, glm::vec3(0.2f));
	//Set the model matrix uniform
	(*gShaders)[gCurrentShaderIndex].SetMat4("model", model);
	//Render the detailed model
	gModel->Draw((*gShaders)[gCurrentShaderIndex]);

	if (shadowType == 0)
	{
		if (showNormals)
		{
			//Change the shader to show normal vectors
			changeShader(9);

			//Set the model matrix uniform
			(*gShaders)[gCurrentShaderIndex].SetMat4("model", model);
			//Render the detailed model
			gModel->Draw((*gShaders)[gCurrentShaderIndex]);
		}

		//Change the shader for the reflective cube
		changeShader(6);
	}
	else if (shadowType == 1)
	{
		//Switch to the simple depth shader
		changeShader(11);
	}
	else if (shadowType == 2)
	{
		//Switch to the simple point depth shader
		changeShader(16);
	}

	//Re-enable face culling
	glEnable(GL_CULL_FACE);

	//Render the reflective cube
	gReflectiveCubeModel->Draw((*gShaders)[gCurrentShaderIndex]);

	if (shadowType == 0)
	{
		//Change the shader for the refractive cube
		changeShader(7);
	}

	//Render the refractive cube
	gRefractiveCubeModel->Draw((*gShaders)[gCurrentShaderIndex]);

	//Disable face culling
	glDisable(GL_CULL_FACE);

	if (shadowType == 0)
	{
		//Change the depth function to allow skybox depth values to pass
		glDepthFunc(GL_LEQUAL);
		//Disable writing to the depth buffer
		glDepthMask(GL_FALSE);

		//Use the skybox shader
		changeShader(5);

		//Set the view and projection matrices for the skybox
		glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); //Remove translation from the view matrix
		(*gShaders)[gCurrentShaderIndex].SetMat4("view", skyboxView);
		(*gShaders)[gCurrentShaderIndex].SetMat4("projection", projection);

		//Render the skybox cube
		gSkyboxCube->Draw((*gShaders)[gCurrentShaderIndex]);

		//Reset the depth function
		glDepthFunc(GL_LESS);

		//Use the main shader
		changeShader(1);
	}
	else if (shadowType == 1)
	{
		//Switch to the transparent depth shader
		changeShader(14);

		//Set the light space matrix uniform
		(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);
	}
	else if (shadowType == 2)
	{
		//Switch to the transparent point depth shader
		changeShader(17);
	}

	//Create a sorted map of the glass planes based on distance from the camera
	std::map<float, PlaneModel*> sorted;
	for (GLuint i = 0; i < 5; ++i)
	{
		float distance = glm::length(gCamera->position - gGlassPlaneModels[i].getPosition());
		sorted[distance] = &gGlassPlaneModels[i];
	}

	//Render the glass planes in back-to-front order
	for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
	{
		it->second->Draw((*gShaders)[gCurrentShaderIndex]);
	}

	//Re-enable face culling
	glEnable(GL_CULL_FACE);

	if (shadowType == 0)
	{
		//Re-enable writing to the depth buffer
		glDepthMask(GL_TRUE);

		//Draw an outline effect around the cubes
		if (outlineEffectEnabled)
		{
			//Setup the stencil buffer operations for the outline effect
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			//Disable writing to the stencil buffer
			glStencilMask(0x00);

			//Disable the depth buffer operations
			glDisable(GL_DEPTH_TEST);

			//Use the border shader
			changeShader(4);

			//Render the scaled up cubes for the outline effect
			for (int i = 0; i < 2; ++i)
			{
				//Save the original scale
				glm::vec3 originalScale = gCubeModels[i].getScale();

				//Set the new scale
				gCubeModels[i].setScale(originalScale * 1.1f);

				//Draw the cube
				gCubeModels[i].Draw((*gShaders)[gCurrentShaderIndex]);

				//Restore the original scale
				gCubeModels[i].setScale(originalScale);
			}

			//Re-enable depth buffer writing
			glEnable(GL_DEPTH_TEST);

			//Reset the stencil buffer settings
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);

			//Change the shader back to the main shader
			changeShader(1);
		}
	}
}

void renderSpaceScene(float timeValue, glm::mat4 view, glm::mat4 projection, int shadowType)
{
	if (shadowType == 0)
	{
		//Enable the default shader
		changeShader(1);

		//Ensure the stencil buffer is configured to always pass
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		//Bind the shadow map texture
		glActiveTexture(GL_TEXTURE25);
		glBindTexture(GL_TEXTURE_2D, gShadowMapTexture);
		//Load the shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMap", 25);
		//Load the light space matrix uniform
		(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);

		//Bind the point light shadow map cube texture
		glActiveTexture(GL_TEXTURE26);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gPointLightShadowMapCubeTexture);
		//Load the point light shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMapPoint", 26);

		//Update the spotlight position and direction uniforms
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.position", gCamera->position);
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.direction", gCamera->direction);
	}

	//Declare the model matrix for the planet
	glm::mat4 model = glm::mat4(1.0f);
	//Apply transformations to the model matrix
	model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
	model = glm::scale(model, glm::vec3(4.0f));

	//Set the model matrix uniform
	(*gShaders)[gCurrentShaderIndex].SetMat4("model", model);

	//Render the planet model
	gPlanetModel->Draw((*gShaders)[gCurrentShaderIndex]);

	if (shadowType == 0)
	{
		//Switch to the instance shader
		changeShader(10);

		//Update the spotlight position and direction uniforms for instance shader
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.position", gCamera->position);
		(*gShaders)[gCurrentShaderIndex].SetVec3("spotLight.direction", gCamera->direction);

		//Bind the shadow map texture
		glActiveTexture(GL_TEXTURE25);
		glBindTexture(GL_TEXTURE_2D, gShadowMapTexture);
		//Load the shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMap", 25);
		//Load the light space matrix uniform
		(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);

		//Bind the point light shadow map cube texture
		glActiveTexture(GL_TEXTURE26);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gPointLightShadowMapCubeTexture);
		//Load the point light shadow map texture uniform
		(*gShaders)[gCurrentShaderIndex].SetInt("shadowMapPoint", 26);
	}
	else if (shadowType == 1)
	{
		//Switch to the instanced depth shader
		changeShader(13);

		//Set the light space matrix uniform
		(*gShaders)[gCurrentShaderIndex].SetMat4("lightSpaceMatrix", gLightSpaceMatrix);
	}
	else if (shadowType == 2)
	{
		//Switch to the instanced point depth shader
		changeShader(19);
	}

	//Draw the asteroids using instanced rendering
	for(GLuint i = 0; i < gAsteroidModel->GetMeshes().size(); i++)
	{
		//Bind textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		const auto& textures = gAsteroidModel->GetMeshes()[i].textures;
		for (unsigned int j = 0; j < textures.size(); j++)
		{
			glActiveTexture(GL_TEXTURE0 + j);
			std::string number;
			std::string name = textures[j].type;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++);

			(*gShaders)[gCurrentShaderIndex].SetInt((name + number).c_str(), j);
			glBindTexture(GL_TEXTURE_2D, textures[j].id);
		}
		glActiveTexture(GL_TEXTURE0);

		//Bind the rock mesh VAO
		glBindVertexArray(gAsteroidModel->GetMeshes()[i].GetVAO());

		//Draw the asteroid model
		glDrawElementsInstanced(GL_TRIANGLES, gAsteroidModel->GetMeshes()[i].indices.size(), GL_UNSIGNED_INT, 0, gAsteroidInstanceAmount);
	}

	//Unbind the VAO
	glBindVertexArray(0);
}
