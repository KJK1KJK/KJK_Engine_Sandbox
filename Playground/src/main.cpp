//File for testing external library features before implementing them in the engine
#include <KJK>

#include "Shader.h"
#include "Camera.h"

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

//OpenGl object IDs
GLuint gVBOs[2];
GLuint gVAOs[2];
GLuint gEBOs[2];

//Shader program ID
std::optional<std::array<Shader, 3>> gShaders;

//Texture ID
GLuint gTextures[2];

//Cube positions
std::optional<std::array<glm::vec3, 10>> gCubePositions;

//Light source position
glm::vec3 gLightPos = glm::vec3(1.0f);

//Camera object
Camera* gCamera;

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
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//Calculate delta time
					float currentFrame = static_cast<float>(SDL_GetTicks()) / 1000.0f;
					deltaTime = currentFrame - lastFrame;
					lastFrame = currentFrame;

					//Get keyboard state
					const bool* keyState = SDL_GetKeyboardState(NULL);

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

					//Handle camera movement input
					gCamera->HandleInput(e, deltaTime, mouseCaptured, keyState);
				}

				//Set the clear color
				glClearColor(0.2f, 0.f, 0.2f, 1.0f);
				//Clear the color buffer
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//Set to polygon wireframe mode
				//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				//Calculate the time value
				float timeValue = static_cast<float>(SDL_GetTicks()) / 1000.0f;

				//Bind the textures
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gTextures[0]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gTextures[1]);
				
				//Define a view matrix
				glm::mat4 view = gCamera->GetViewMatrix();

				//Define a projection matrix
				glm::mat4 projection = glm::mat4(1.0f);
				projection = glm::perspective(glm::radians(gCamera->fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

				//Switch to the light source shader
				(*gShaders)[2].Use();

				//Apply the transformation matrices to the shader
				(*gShaders)[2].SetMat4("view", view);
				(*gShaders)[2].SetMat4("projection", projection);

				//Move the lightning source on an orbit
				gLightPos.x = 10.5f * sin(timeValue);
				gLightPos.z = 10.5f * cos(timeValue);

				//Set the model matrix for the light source cube
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, gLightPos);
				//Scale down the light source cube
				model = glm::scale(model, glm::vec3(0.2f));
				//Apply the model matrix to the shader
				(*gShaders)[2].SetMat4("model", model);

				//Bind the light source VAO
				glBindVertexArray(gVAOs[1]);
				//Draw the light source cube
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

				//Use the defined shader program for the objects
				(*gShaders)[1].Use();

				//Update the light position uniform
				(*gShaders)[1].SetVec3("lightPos", gLightPos);

				//Apply the transformation matrices to the object shader
				(*gShaders)[1].SetMat4("view", view);
				(*gShaders)[1].SetMat4("projection", projection);

				//Bind the cube VAO
				glBindVertexArray(gVAOs[0]);

				//Iterate over cube position to draw 10 objects that rotate
				for (GLuint i = 0; i < 10; i++)
				{
					//Define a model matrix for the object and move each to the correct position
					glm::mat4 model = glm::mat4(1.0f);
					model = glm::translate(model, (*gCubePositions)[i]);

					//Calculate the angle based on the i value
					float angle = 20.0f * (i+1);

					//Rotate the object over time
					model = glm::rotate(model, timeValue * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

					//Apply the model matrix to the object shader
					(*gShaders)[1].SetMat4("model", model);

					//Draw an object
					glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				}

				//Unbind the VAO
				glBindVertexArray(0);

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
		gWindow = SDL_CreateWindow("LearnOpenGL", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

	//Generate VAOs
	glGenVertexArrays(2, gVAOs);
	//Generate VBOs
	glGenBuffers(2, gVBOs);
	//Generate EBOs
	glGenBuffers(2, gEBOs);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);

	//Set of vertices for a cube
	GLfloat vertices[] =
	{
		//Vertex Positions     //Colors           //Texture    //Normal
		//Front side
		 0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  2.0f, 2.0f,   0.0f,  0.0f,  1.0f,  //Top Right
		 0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  2.0f, 0.0f,   0.0f,  0.0f,  1.0f,  //Bottom Right
		-0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f,   0.0f,  0.0f,  1.0f,  //Bottom Left
		-0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f,  0.0f, 2.0f,   0.0f,  0.0f,  1.0f,  //Top Left
		//Back Side											     	   
		 0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f,  2.0f, 2.0f,   0.0f,  0.0f, -1.0f,  //Top Right
		 0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,  2.0f, 0.0f,   0.0f,  0.0f, -1.0f,  //Bottom Right
		-0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f,  0.0f, -1.0f,  //Bottom Left
		-0.5f,  0.5f, -0.5f,   0.5f, 0.5f, 0.5f,  0.0f, 2.0f,   0.0f,  0.0f, -1.0f,   //Top Left
		//Left Side											     	   
		-0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  2.0f, 2.0f,  -1.0f,  0.0f,  0.0f,  //Top Right
		-0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,  2.0f, 0.0f,  -1.0f,  0.0f,  0.0f,  //Bottom Right
		-0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f,  -1.0f,  0.0f,  0.0f,  //Bottom Left
		-0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,  0.0f, 2.0f,  -1.0f,  0.0f,  0.0f,  //Top Left
		//Right Side										     	   
		 0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 1.0f,  2.0f, 2.0f,   1.0f,  0.0f,  0.0f,  //Top Right
		 0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,  2.0f, 0.0f,   1.0f,  0.0f,  0.0f,  //Bottom Right
		 0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   1.0f,  0.0f,  0.0f,  //Bottom Left
		 0.5f,  0.5f, -0.5f,   0.5f, 0.5f, 0.5f,  0.0f, 2.0f,   1.0f,  0.0f,  0.0f,   //Top Left
		//Top Side											     
		 0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,  2.0f, 2.0f,   0.0f,  1.0f,  0.0f,  //Top Right
		 0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,  2.0f, 0.0f,   0.0f,  1.0f,  0.0f,  //Bottom Right
	    -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f,   0.0f,  1.0f,  0.0f,  //Bottom Left
	    -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f,  0.0f, 2.0f,   0.0f,  1.0f,  0.0f,  //Top Left
	    //Bottom Side										     			 
		 0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 1.0f,  2.0f, 2.0f,   0.0f, -1.0f,  0.0f,  //Top Right
		 0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,  2.0f, 0.0f,   0.0f, -1.0f,  0.0f,  //Bottom Right
	    -0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f, -1.0f,  0.0f,  //Bottom Left
	    -0.5f, -0.5f,  0.5f,   0.5f, 0.5f, 0.5f,  0.0f, 2.0f,   0.0f, -1.0f,  0.0f   //Top Left
	};
	GLuint indices[] =
	{
		 0,  1,  2,  //First Front Triangle
		 0,  2,  3,  //Second Front Triangle
		 4,  5,  6,  //First Back Triangle
		 4,  6,  7,  //Second Back Triangle
		 8,  9, 10,  //First Left Triangle
		 8, 10, 11,  //Second Left Triangle
		12, 13, 14,  //First Right Triangle
		12, 14, 15,  //Second Right Triangle
		16, 17, 18,  //First Top Triangle
		16, 18, 19,  //Second Top Triangle
		20, 21, 22,  //First Bottom Triangle
		20, 22, 23   //Second Bottom Triangle
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	//Set of vertices for a triangle
	GLfloat vertices2[] =
	{
		 0.5f,  0.5f, 0.0f,   //Top Right
		 0.5f, -0.5f, 0.0f,   //Bottom Right
		-0.5f,  0.5f, 0.0f,   //Top Left
	};
	GLuint indices2[] =
	{
		0, 1, 2
	};

	//Bind VAO for a light object
	glBindVertexArray(gVAOs[1]);

	//Bind VBO for a light object
	glBindBuffer(GL_ARRAY_BUFFER, gVBOs[0]);

	//Bind EBO for a light object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBOs[0]);

	//Inform OpenGL how to interpret the vertex data for a light object
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	//Define positions to draw the cubes to
	gCubePositions.emplace(std::array<glm::vec3, 10>
	{
		glm::vec3( 0.0f,  0.0f,  0.0f),
		glm::vec3( 2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3( 2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3( 1.3f, -2.0f, -2.5f),
		glm::vec3( 1.5f,  2.0f, -2.5f),
		glm::vec3( 1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	});

	//Define the light position
	gLightPos = glm::vec3(1.2f, 1.0f, 2.0f);

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
		Shader("assets/shaders/shader.vert", "assets/shaders/shader1.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/shader2.frag"),
		Shader("assets/shaders/shader.vert", "assets/shaders/lightSourceShader.frag")
	});

	//Load the wood container texture
	SDL_Surface* containerSurface = IMG_Load("assets/container.jpg");
	if (containerSurface == nullptr)
	{
		KJK_ERROR("Failed to load texture image: {0}", SDL_GetError());
		success = false;
	}
	else
	{
		//Generate a texture
		glGenTextures(1, &gTextures[0]);
		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, gTextures[0]);

		//Generate the texture using the loaded surface data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, containerSurface->w, containerSurface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, containerSurface->pixels);

		//Set the texture wrapping/filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);

		//Free the loaded surface
		SDL_DestroySurface(containerSurface);
	}

	//Load the wood container texture
	SDL_Surface* faceSurface = IMG_Load("assets/awesomeface.png");
	if (faceSurface == nullptr)
	{
		KJK_ERROR("Failed to load texture image: {0}", SDL_GetError());
		success = false;
	}
	else
	{
		//Flip the surface vertically
		if (!SDL_FlipSurface(faceSurface, SDL_FLIP_VERTICAL))
		{
			KJK_ERROR("Failed to flip surface: {0}", SDL_GetError());
		}

		//Generate a texture
		glGenTextures(1, &gTextures[1]);
		//Bind the texture
		glBindTexture(GL_TEXTURE_2D, gTextures[1]);

		//Generate the texture using the loaded surface data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, faceSurface->w, faceSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, faceSurface->pixels);

		//Set the texture wrapping/filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);

		//Free the loaded surface
		SDL_DestroySurface(faceSurface);
	}

	//Set the texture uniforms
	(*gShaders)[1].Use();
	(*gShaders)[1].SetInt("texture1", 0);
	(*gShaders)[1].SetInt("texture2", 1);

	//Set the color uniforms for the light on the objects
	(*gShaders)[1].SetVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
	(*gShaders)[1].SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	(*gShaders)[1].SetVec3("lightPos", gLightPos);
	(*gShaders)[1].SetVec3("viewPos", gCamera->position);
	(*gShaders)[1].SetFloat("ambientStrength", 0.1f);
	(*gShaders)[1].SetFloat("specularStrength", 0.5f);

	//Set the color uniforms for the light source
	(*gShaders)[2].Use();
	(*gShaders)[2].SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

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
	
	//Destroy texture
	glDeleteTextures(2, gTextures);

	//Delete the camera
	delete gCamera;

	//Destroy window
	if (gWindow != nullptr)
	{
		SDL_DestroyWindow(gWindow);
		gWindow = nullptr;
	}

	//Quit SDL subsystems
	SDL_Quit();
}