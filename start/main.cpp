#include <glew.h> // used for cross version opengl
#include <glfw3.h> // GLFW helper library handle input or windows...
#include <stdio.h>
#include <glm.hpp>
#include "BitMap/BitMap.h"
#include "detail/func_common.hpp"

#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/CommonStructs.h"
#include "Graphics/FakeShaders/VertexShader.h"
#include "Graphics/FromVStoPS/FrameBuffer.h"
#include "Graphics/FromVStoPS/PriAsm.h"
#include "Graphics/Camera.h"

#include "Utilities/WinTiming.h"
#include "Utilities/Profiling.h"
#include "SimplexNoise.h"

#include "Graphics/SRDef.h"
#include "Graphics/Graphic.h"

#include "Graphics/Objects/Mesh.h"
#include "Graphics/Objects/Material.h"
#include "Graphics/Objects/Texture.h"
#include "Graphics/Objects/ResourceManager.h"


using namespace SYSR;
using namespace glm;

GLuint texture;
GLuint vao;
GLuint shader_programme;
GLuint TextureID;
GLuint pboIds[2];

unsigned int ChannelCount = 4; 
unsigned int DataSize = SCREENWIDTH*SCREENHEIGHT*ChannelCount;

GLFWwindow* window;

// Some openGl stuff
void GLInitilize()
{
	float points[] = {
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,

		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};
	// Vertex buffer
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), points, GL_STREAM_DRAW);

	// Vertex array object
	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Texture quad for rendering
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->GetData());
	glBindTexture(GL_TEXTURE_2D, 0);

	// Pixel buffer to modify texture data
	glGenBuffers(2, pboIds);
	glBindVertexArray(vao);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[0]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, SCREENWIDTH * SCREENHEIGHT * 4, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Read OpenGL Shaders
	const char* vertex_shader =
		"#version 430\n"
		"in vec3 vp;"
		"out vec2 UV;"
		"void main () {"
		"  gl_Position = vec4 (vp, 1.0);"
		"	UV = (vp.xy+vec2(1.0,1.0))/2.0;"
		"}";
	const char* fragment_shader =
		"#version 430\n"
		"out vec3 frag_colour;"
		"in vec2 UV;"
		"layout (binding = 0) uniform sampler2D renderedTexture;"
		"void main () {"
		"  frag_colour = texture2D(renderedTexture,UV).rgb;"//vec3 (1,0,1.0);"//vec4 (UV.y, UV.x, 0.0, 1.0)
		"}";

	// Create and compile Shaders
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	// Apply Shaders
	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

}


#pragma region Contents
Mesh *s_pMesh;
Material *s_pMaterial;
Texture *s_pTexture;
void SceneInitilize()
{

	ResourceManager::CreateInstance();

	ResourceManager::GetInstance()->LoadTexturesFromPath("../Textures/");
	ResourceManager::GetInstance()->LoadMaterialsFromPath("../Materials/");

	s_pMesh = new Mesh();
	s_pMaterial = new Material();

	//s_pTexture = new Texture("../Textures/spnza_bricks_a_diff.bmp");
	//s_pMaterial->SetTexture(s_pTexture);

	//s_pMesh->SetMaterial(s_pMaterial);

}

float left_dir = 0.0f;
float up_dir = -1.0f;
const float offset = 0.5f;
void SceneDraw()
{	


	s_pMesh->Draw();
//	s_pMesh_front->Draw();	
}
#pragma endregion

void PrintVec2(const char *name, vec2 &vec2)
{
	printf("%s :%f,%f \n", name, vec2.x, vec2.y);
}

#pragma region Input handling

// Input helpers
int keyStates[400];
int mouseState[8];
vec2 curMousePos,lastMousePos(400,300);

#define KEY_HELD 1
#define KEY_RELEASED 0

// Document says do not use this to impelement camera control
void SetMousePos(double x,double y )
{
	glfwSetCursorPos(window,x,y);
}

 void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action != GLFW_RELEASE)
	{
		keyStates[key] = KEY_HELD;
	}
	else
	{
		keyStates[key] = KEY_RELEASED;
	}
}
void mouse_callback(GLFWwindow* window, int key, int action, int mods)
{
	if (action != GLFW_RELEASE)
	{
		mouseState[key] = KEY_HELD;
	}
	else
	{
		mouseState[key] = KEY_RELEASED;
	}
}
// left up corner is(0,0)
void mousePos_callback(GLFWwindow* window, double x, double y)
{
	curMousePos.x = x; curMousePos.y = y;
	vec2 offset = curMousePos - lastMousePos;
	lastMousePos = curMousePos;

	static const float factor = 0.001f;
	offset *= factor;

	//PrintVec2("offset", lastMousePos);

	// get the axis to rotate around the x-axis. 
	vec3 Axis = cross(Camera::viewVector - Camera::cameraPos, vec3(0, 1, 0));
	// To be able to use the quaternion conjugate, the axis to
	// rotate around must be normalized.
	Axis = normalize(Axis);

	pCamera->RotateCamera(-offset.y, Axis.x, Axis.y, Axis.z);// rotate around local x axis
	pCamera->RotateCamera(-offset.x, 0, 1, 0);//rotate around local y axis

}

bool GetKey(int key)
{
	return keyStates[key] == KEY_HELD;
}
bool GetMouse(int key)
{
	return mouseState[key] == KEY_HELD;
}

void InputUpdates()
{
	using namespace glm;
	float speed = 0.1f;

	if (GetKey(GLFW_KEY_LEFT_SHIFT))
		speed = 1.0f;
	vec3 forwarDir = Camera::GetForwardDir();

	vec3 move(0,0,0);
	if (GetKey(GLFW_KEY_W))
	{
		move += Camera::GetForwardDir() * speed;
	}
	if (GetKey(GLFW_KEY_S))
	{
		move -= Camera::GetForwardDir() * speed;
	}
	if (GetKey(GLFW_KEY_A))
	{
		move -= Camera::GetLeftDir() * speed;
	}
	if (GetKey(GLFW_KEY_D))
	{
		move += Camera::GetLeftDir() * speed;
	}
	if (GetKey(GLFW_KEY_Q))
	{
		move -= vec3(0, 1, 0) * speed;
	}
	if (GetKey(GLFW_KEY_E))
	{
		move += vec3(0,1,0) * speed;
	}
	
	Camera::cameraPos += move;
	Camera::viewVector = Camera::cameraPos + forwarDir * 10.0f;

	pCamera->Update();

	//if (GetMouse(GLFW_MOUSE_BUTTON_LEFT))
}


#pragma endregion

void DrawLoop()
{
	//	PROFILE_UNSCOPED("Graphic")

	//double diff = Timing::WinTiming::getCurenntFrameTime_ms() - Timing::WinTiming::getLastFrameTime_ms();
	//Timing::WinTiming::setLastFrameTime_ms(Timing::WinTiming::getCurenntFrameTime_ms());


	//printf("%f FPS \n", 1.0 / diff);

	// Soft rendering logic
	bitmap->Clear_White();//Clear(0, 255, 255, 255);
	CleanZBuffer();

	//Graphics_Draw();
	SceneDraw();

	//Test
	//bitmap->ImageProcess();

	// Explanation:http://www.songho.ca/opengl/gl_pbo.html#create
	static int index = 0;
	int nextIndex = 0;
	index = (index + 1) % 2;
	nextIndex = (index + 1) % 2;

	// Change texture
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[index]);
	glBindTexture(GL_TEXTURE_2D, texture);//Should have use GL_RAGB???
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREENWIDTH, SCREENHEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Get pixel buffer pointer and update data
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[nextIndex]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, DataSize, 0, GL_STREAM_DRAW);
	GLubyte* dst = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	if (dst)
	{
		memcpy(dst, bitmap->m_data, sizeof(unsigned char) * DataSize);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release pointer to mapping buffer
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Normal GL draw methods
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader_programme);
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// update other events like input handling 
	glfwPollEvents();


	// put the stuff we've been drawing onto the display
	glfwSwapBuffers(window);

}

int main () {
	// start GL context and O/S window using the GLFW helper library
	if (!glfwInit ()) {
		fprintf (stderr, "ERROR: could not start GLFW3\n");
		return 1;
	} 

	window = glfwCreateWindow (SCREENWIDTH * 2.0f,SCREENHEIGHT * 2.0f, "Software rendering", NULL, NULL);
	if (!window) {
		fprintf (stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent (window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetCursorPosCallback(window, mousePos_callback);
	
	// used for camera control
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit ();

	Graphics_Initilize();// SoftRendering
	GLInitilize();//OpenGL stuff


	SceneInitilize();//Scene
	pCamera->SetLookAt(vec3(0, 0, 1), vec3(0, 0,0), vec3(0, 1, 0));

	// Input helpers
	for (int i = 0; i < 400; i++)
		keyStates[i] = 0;
	
	float inputCounter = 0.0f;
	while (!glfwWindowShouldClose (window)) 
	{

		DrawLoop();

		double delta = Timing::WinTiming::getCurenntFrameTime_ms() - Timing::WinTiming::getLastFrameTime_ms();
		Timing::WinTiming::setLastFrameTime_ms(Timing::WinTiming::getCurenntFrameTime_ms());

		printf("%f FPS \n", 1.0 / delta);

		inputCounter += delta;
		if (inputCounter >= 0.02f)
		{
			InputUpdates();
			inputCounter = 0.0f;
		}
			
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	

	Graphic_ShutDown();

	PROFILE_PRINT_RESULTS
		system("Pause");
	return 0;
}