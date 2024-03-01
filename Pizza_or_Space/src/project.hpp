#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <list>
#include <chrono>
#include <thread>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"
#include <vector>
#include <numeric>
#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "objload.h"
#include "SOIL/stb_image_aug.h"
#include <Windows.h>
#include <iomanip> 
#include <sstream>
#include "SOIL/SOIL.h"
#include "SkyboxVertices.h"

using namespace std;
bool firstMouse = true;
bool rKeyWasPressed = false;
float lastX;
float lastY;
float pitch;
float yaw;
float n = 0.05;
float f = 1000;

namespace texture {
	GLuint earth;
	GLuint mars;
	GLuint jupiter;
	GLuint neptune;
	GLuint saturn;
	GLuint mercury;
	GLuint venus;
	GLuint uranus;
	GLuint moon;
	GLuint fighter;
	GLuint asteroid;
	GLuint sun;
	GLuint metal;
	GLuint earth_nmap;
	GLuint pizza;
}


GLuint program;
GLuint programSun;
GLuint programTex;
GLuint programEarth;
GLuint programProcTex;
GLuint skyboxProgram;
GLuint cubemapTexture;
GLuint programUnmap;
GLuint skyboxVAO, skyboxVBO;
Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext stationContext;
Core::RenderContext sphereContext;
Core::RenderContext pizzaContext;
GLuint empty_nmap_texture = -1;
glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);

glm::vec3 spaceshipPos = glm::vec3(-25.f, 0.5f, 0.f);
glm::vec3 spaceshipDir = glm::vec3(1.f, 0.f, 0.f);
GLuint VAO, VBO;

float aspectRatio = 1.777777777777778f;
glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8);

std::vector<glm::vec3> asteroidPositions;

struct Pizza {
	glm::vec3 position;
	bool collected;
	float rotationAngleX; // Rotation angle around the X axis
	float rotationAngleY; // Rotation angle around the Y axis
	float rotationAngleZ; // Rotation angle around the Z axis
};


std::vector<Pizza> pizzas;




struct Planet {
	glm::vec3 startPosition;
	glm::vec3 modelScale;
	bool isActivated;
	GLuint textureID;
	std::string name;
	glm::vec3 currentPlanetPos;
	bool interacted;
	GLuint nmap_texture;
	bool isNmap;

};

std::vector<Planet> planets;
std::vector<Planet*> targetPlanets;

struct Asteroid {
	glm::vec3 startPosition;
	glm::vec3 modelScale;
	glm::vec3 currentAsteroidPos;
	bool interacted;
	
};

std::vector<Asteroid> asteroids;


glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{

	glm::mat4 perspectiveMatrix;

	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
		0.,0.,-1.,0.,
		});


	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectTexture(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID) {
	program = programTex;
	if (textureID == texture::sun)
		program = programSun;
	glUseProgram(program);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, glm::value_ptr(transformation));
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glUniform3f(glGetUniformLocation(program, "lightPos"), 0, 0, 0);
	Core::SetActiveTexture(textureID, "colorTexture", program, 0);
	// Ustawianie uniformów dla shadera
	if (program == programTex) {
		glUniform1f(glGetUniformLocation(programTex, "metallic"), 0.3);
		glUniform1f(glGetUniformLocation(programTex, "roughness"), 0.5);
		glUniform3fv(glGetUniformLocation(programTex, "albedo"), 1, glm::value_ptr(glm::vec3(1.0, 0.5, 0.31)));
	}

	Core::DrawContext(context);
}




void drawObjectTextureWithNMAP(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint textureID, GLuint nmap_texture) {
	glUseProgram(programUnmap);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programUnmap, "transformation"), 1, GL_FALSE, glm::value_ptr(transformation));
	glUniformMatrix4fv(glGetUniformLocation(programUnmap, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glUniform3f(glGetUniformLocation(programUnmap, "lightPos"), 0, 0, 0);
	Core::SetActiveTexture(textureID, "colorTexture", programUnmap, 0);

	glUniform1f(glGetUniformLocation(programUnmap, "metallic"), 0.3);
	glUniform1f(glGetUniformLocation(programUnmap, "roughness"), 0.5);
	glUniform3fv(glGetUniformLocation(programUnmap, "albedo"), 1, glm::value_ptr(glm::vec3(1.0, 0.5, 0.31)));
	glUniform1i(glGetUniformLocation(programUnmap, "normalMap"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, nmap_texture);


	Core::DrawContext(context);
}


void drawObjectSkybox() {
	glDepthMask(GL_FALSE);
	glUseProgram(skyboxProgram);
	glm::mat4 viewMatrix = glm::mat4(glm::mat3(createCameraMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Get perspective matrix
	glm::mat4 projectionMatrix = createPerspectiveMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glBindVertexArray(skyboxVAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
}


void renderScene(GLFWwindow* window)
{
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	glm::mat4 transformation;
	float time = glfwGetTime();
	drawObjectSkybox();
	//sun
	drawObjectTexture(sphereContext, glm::mat4(), texture::sun);

	float rot = 1.f;


	for (auto& planet : planets) {


		
		glm::mat4 modelMatrix = glm::eulerAngleY(time / rot) * glm::translate(planet.startPosition) * glm::eulerAngleY(time) * glm::scale(planet.modelScale);
		drawObjectTextureWithNMAP(sphereContext, modelMatrix, planet.textureID, texture::earth_nmap);
		planet.currentPlanetPos = glm::vec3(modelMatrix * glm::vec4(0, 0, 0, 1.0f));
		rot += 0.5f;
	}

	
	drawObjectTexture(sphereContext,
		glm::eulerAngleY(time / 2.0f) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)), texture::moon);
	
	drawObjectTexture(stationContext,
		glm::translate(glm::vec3(-25.f, -0.f, 0)) * glm::scale(glm::vec3(0.5f)),
		texture::metal
	);



	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});
	
	// Model adjustment
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
	glm::mat4 rotationMatrix2 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
	glm::mat4 spaceshipModelMatrix = glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * rotationMatrix * rotationMatrix2 * glm::scale(glm::vec3(0.35f));

	drawObjectTexture(shipContext, spaceshipModelMatrix, texture::fighter);
	glUseProgram(0);
	glUseProgram(programTex);
	for (auto& pizza : pizzas) {
		if (!pizza.collected) {
			float distanceToPizza = glm::distance(cameraPos, pizza.position);

			// Assuming the pizza is collected when it's close to the spaceship
			if (distanceToPizza < 0.5f) {
				// Player collected the pizza
				pizza.collected = true;
				// Perform any additional actions here (e.g., score increment)
			}
			else {

				// Draw the pizza only if it's not collected
				drawObjectTexture(pizzaContext,
					glm::translate(pizza.position) *
					glm::rotate(glm::mat4(1.0f), glm::radians(pizza.rotationAngleX), glm::vec3(1.0f, 0.0f, 0.0f)) *
					glm::rotate(glm::mat4(1.0f), glm::radians(pizza.rotationAngleY), glm::vec3(0.0f, 1.0f, 0.0f)) *
					glm::rotate(glm::mat4(1.0f), glm::radians(pizza.rotationAngleZ), glm::vec3(0.0f, 0.0f, 1.0f)) *
					glm::scale(glm::vec3(0.0035f)) *
					glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
					texture::pizza);
			}
		}
	}

	glUseProgram(0);
	glfwSwapBuffers(window);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}


void initializePlanets() {

	planets.push_back(Planet{ glm::vec3(1.5f, 0, 0), glm::vec3(0.1f), false, texture::mercury, "Mercury", glm::vec3(1.5f, 0, 0), false, empty_nmap_texture, FALSE });
	planets.push_back(Planet{ glm::vec3(2.3f, 0, 0), glm::vec3(0.15f), false, texture::venus, "Venus", glm::vec3(2.3f, 0, 0), false, empty_nmap_texture, FALSE });
	planets.push_back(Planet{ glm::vec3(4.f, 0, 0), glm::vec3(0.25f), false, texture::earth, "Earth", glm::vec3(4.f, 0, 0), false, texture::earth_nmap, TRUE });
	planets.push_back(Planet{ glm::vec3(6.6f, 0, 0), glm::vec3(0.27f), false, texture::mars, "Mars", glm::vec3(6.6f, 0, 0), false, empty_nmap_texture, FALSE });
	planets.push_back(Planet{ glm::vec3(7.4f, 0, 0), glm::vec3(0.43f), false, texture::jupiter, "Jupiter", glm::vec3(7.4f, 0, 0), false, empty_nmap_texture, FALSE });
	planets.push_back(Planet{ glm::vec3(11.f, 0, 0), glm::vec3(0.47f), false, texture::saturn, "Saturn", glm::vec3(11.f, 0, 0), false, empty_nmap_texture, FALSE });
	planets.push_back(Planet{ glm::vec3(12.f, 0, 0), glm::vec3(0.32f), false, texture::uranus, "Uranus", glm::vec3(12.f, 0, 0), false, empty_nmap_texture, FALSE });
	planets.push_back(Planet{ glm::vec3(14.f, 0, 0), glm::vec3(0.3f), false, texture::neptune, "Neptune", glm::vec3(14.f, 0, 0), false, empty_nmap_texture, FALSE });
	for (auto& planet : planets) {
		for (int i = 0; i < 3; ++i) { // Generate 3 pizza objects near each planet
			float angle = glm::radians(static_cast<float>(rand() % 360));// Random angle around the planet
			float distance = 0.5f + static_cast<float>(rand() % 5); // Random distance from the planet
			glm::vec3 pizzaPosition = planet.startPosition + glm::vec3(cos(angle) * distance, 0.0f, sin(angle) * distance);
			pizzas.push_back(Pizza{ pizzaPosition, false });
		}
	}
}

GLuint loadCubemap(std::vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height;
	unsigned char* image;

	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		if (image)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			SOIL_free_image_data(image);
		}
		else
		{
			std::cout << "failed: " << faces[i] << std::endl;
			SOIL_free_image_data(image);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void init(GLFWwindow* window){


	std::vector<std::string> faces
	{
		"./textures/skybox/bkg1_right1.png",
		"./textures/skybox/bkg1_left2.png",
		"./textures/skybox/bkg1_top3.png",
		"./textures/skybox/bkg1_bottom4.png",
		"./textures/skybox/bkg1_front5.png",
		"./textures/skybox/bkg1_back6.png",

	};
	cubemapTexture = loadCubemap(faces);


	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	program = shaderLoader.CreateProgram("shaders/shader.vert", "shaders/shader.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programEarth = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programProcTex = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_sun.vert", "shaders/shader_sun.frag");
	/*-------------------------skybox_start---------------------------*/
	skyboxProgram = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	/*---------------------------skybox_end-------------------------*/
	programUnmap = shaderLoader.CreateProgram("shaders/shader_unmap.vert", "shaders/shader_unmap.frag");

	texture::earth = Core::LoadTexture("textures/earth.png");
	texture::fighter = Core::LoadTexture("textures/fighter_texture.jpg");
	texture::moon = Core::LoadTexture("textures/moon.jpg");
	texture::jupiter = Core::LoadTexture("textures/jupiter.jpg");
	texture::mars = Core::LoadTexture("textures/mars.jpg");
	texture::neptune = Core::LoadTexture("textures/neptune.jpg");
	texture::uranus = Core::LoadTexture("textures/uranus.jpg");
	texture::mercury = Core::LoadTexture("textures/mercury.jpg");
	texture::saturn = Core::LoadTexture("textures/saturn.jpg");
	texture::venus = Core::LoadTexture("textures/venus.jpg");
	texture::asteroid = Core::LoadTexture("textures/asteroid.jpg");
	texture::sun = Core::LoadTexture("textures/sun.jpg");
	texture::metal = Core::LoadTexture("textures/metal.jpg");
	texture::pizza = Core::LoadTexture("textures/pizza.png");

	texture::earth_nmap = Core::LoadTexture("textures/earth_normalmap.png");


	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/fighter.obj", shipContext);
	loadModelToContext("./models/station.obj", stationContext);
	loadModelToContext("./models/Pizza.obj", pizzaContext);

	initializePlanets();
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraDir = glm::normalize(direction);
}


void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}


//obsluga wejscia
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.003f;
	float moveSpeed = 0.007f;
	//static bool rKeyPressedLastFrame = false;
	

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		moveSpeed *= 2.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		spaceshipPos += spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		spaceshipPos -= spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	void mouse_callback(GLFWwindow * window, double xpos, double ypos);
	glfwSetCursorPosCallback(window, mouse_callback);
	//bool rKeyPressedThisFrame = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

	cameraPos = spaceshipPos - 1.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.5f;

	spaceshipDir = cameraDir;

	for (auto& pizza : pizzas) {
		if (!pizza.collected && glm::distance(spaceshipPos, pizza.position) < 0.5f) {
			pizza.collected = true;
		}
	}

}


bool allPizzasCollected() {
	for (const auto& pizza : pizzas) {
		if (!pizza.collected) {
			return false;
		}
	}
	return true; 
}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	for (auto& pizza : pizzas) {
		if (!pizza.collected) {
			pizza.rotationAngleX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
			pizza.rotationAngleY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
			pizza.rotationAngleZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
		}
	}

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		renderScene(window);

		if (allPizzasCollected()) {
			std::cout << "All pizzas collected. Congratulations!!!" << std::endl;
			glfwSetWindowShouldClose(window, true);
			break;
		}

		glfwPollEvents();
	}
}

