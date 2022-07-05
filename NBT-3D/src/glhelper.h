#ifndef GL_HELPER
#define GL_HELPER
#pragma once
#include <glad/glad.h>
#include <KHR/khrplatform.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <map>
#include "stb_image.h"

extern GLFWwindow* WINDOW;
extern int WINDOW_WIDTH, WINDOW_HEIGHT;

extern double mouseX, mouseY, pmouseX, pmouseY, mouseScroll;
extern bool mouseLeft, mouseRight, mouseMiddle;

extern bool* keys;

extern bool USE_TEXTURE;

extern std::map<std::string, unsigned int> CACHE;

/// <summary>
/// Initiates OpenGL through GLFW
/// </summary>
/// <param name="verMajor">Major part of version number</param>
/// <param name="verMinor">Minor part of version number</param>
/// <param name="name">Name of the window to be created</param>
/// <param name="windowWidth">Width of window to be created</param>
/// <param name="windowHeight">Height of window to be created</param>
/// <returns>0 if success, -1 if error</returns>
int START_OPEN_GL(char verMajor, char verMinor, const char* name, int windowWidth, int windowHeight);

void uniM4(unsigned int shader, const GLchar* address, glm::mat4 matrix);
void uniF4(unsigned int shader, const GLchar* address, glm::vec4 vec);
void uniF3(unsigned int shader, const GLchar* address, glm::vec3 vec);
void uniF2(unsigned int shader, const GLchar* address, glm::vec2 vec);
void uniF1(unsigned int shader, const GLchar* address, float a);
void uniF4(unsigned int shader, const GLchar* address, float a, float b, float c, float d);
void uniF3(unsigned int shader, const GLchar* address, float a, float b, float c);
void uniF2(unsigned int shader, const GLchar* address, float a, float b);
void uniI4(unsigned int shader, const GLchar* address, int a, int b, int c, int d);
void uniI3(unsigned int shader, const GLchar* address, int a, int b, int c);
void uniI2(unsigned int shader, const GLchar* address, int a, int b);
void uniI1(unsigned int shader, const GLchar* address, int a);
void uniB(unsigned int shader, const GLchar* address, bool a);

// Returns a translation matrix
glm::mat4 translation(glm::vec3 pos);
// Returns a rotation matrix off of a given matrix
glm::mat4 rotation(glm::mat4 mat, glm::vec3 rot);
// Returns a rotation matrix off of [1]
glm::mat4 rotation(glm::vec3 rot);
// Returns a scalar matrix
glm::mat4 scaled(glm::vec3 scaling);
// Returns a positioned, rotated, and possibly scaled matrix off of given matrix, or [1]
glm::mat4 form(glm::vec3 pos, glm::vec3 scale = glm::vec3(1), glm::vec3 rot = glm::vec3(0), glm::mat4 mat = glm::mat4(1));
// Returns a transform function for a specified rectangle shape
glm::mat4 rect(float x, float y, float w, float h, glm::vec3 rot = glm::vec3(0), glm::mat4 mat = glm::mat4(1));

unsigned int CompileShader(unsigned int type, const std::string& source);

unsigned int loadShader(const char* filepath);

unsigned int getTextureFrom(const char* filepath, int magFilter = GL_LINEAR, int minFilter = GL_LINEAR_MIPMAP_LINEAR, int wrapS = GL_CLAMP_TO_EDGE, int wrapT = GL_CLAMP_TO_EDGE);

unsigned int getHDRTexture(const char* filepath, int magFilter = GL_LINEAR, int minFilter = GL_LINEAR_MIPMAP_LINEAR, int wrapS = GL_CLAMP_TO_EDGE, int wrapT = GL_CLAMP_TO_EDGE);
#endif