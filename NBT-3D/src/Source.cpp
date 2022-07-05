/*
* Unpolished project that loads data from an NBT file.
*/


#define GLM_FORCE_SWIZZLE
#include "glhelper.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Useful for debugging, printing vectors and matrices to the console
#include <glm/gtx/string_cast.hpp>

#define NBT_INCLUDE
#define NBT_SHORTHAND
#define NBT_IGNORE_MUTF
#include "../nbt/nbt.hpp"
#define NBT_GZNBT_INCLUDE
#include "../loadgz/gznbt.h"

#include "NBT3D.h"

float x_rotation = 0, y_rotation = 0, z_rotation = 0, x_rotation_accel = 0, y_rotation_accel = 0, z_rotation_accel = 0, zoom = 5;
int main(int argc, char* argv[]) {

	START_OPEN_GL(4, 6, "OpenGL Template", 1280, 720);

	// Load textures and shaders
	stbi_set_flip_vertically_on_load(true);
	unsigned int HDRI = getHDRTexture("hdr.hdr", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	unsigned int HDRD = getTextureFrom("hdrd.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	unsigned int Shader = loadShader("shader.shader");
	unsigned int SphereShader = loadShader("sphere.shader");
	glUseProgram(Shader);

	nbt::NBT3D obj = nbt::NBT3D();
	if (argc > 1)
		obj = nbt::NBT3D::loadOBJ(argv[1])[0];
	else obj = nbt::NBT3D::loadOBJ("monkey.nbt3d")[0];				// Loads .nbt3d

	unsigned int oVAO = obj.getVAO();
	nbt::NBT3D sphere = nbt::NBT3D::loadOBJ("spheremap.obj")[0];	// AND .obj
	unsigned int sVAO = sphere.getVAO();

	uniB(Shader, "useCol", true);
	uniF4(Shader, "inCol", glm::vec4(0.8, 0.8, 0.8, 1));
	stbi_set_flip_vertically_on_load(false);
	
	double start = glfwGetTime();
	double whole_check = floor(glfwGetTime());
	int frames = 0;
	glUseProgram(Shader);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	obj.uniformMaterial(Shader);
	auto hrdil = glGetUniformLocation(Shader, "hdri");
	auto hrddl = glGetUniformLocation(Shader, "hdrd");
	glProgramUniform1i(Shader, hrdil, 0);
	glProgramUniform1i(Shader, hrddl, 1);
	glDepthFunc(GL_LEQUAL);
	while (!glfwWindowShouldClose(WINDOW)) {

		// Rudimentary frame limit, not recommended to use professionally!
		if (glfwGetTime() - start > 1.0 / 120.0) {

			if (WINDOW_WIDTH == 0 || WINDOW_HEIGHT == 0) // If window is minimized...
				continue;
			
			// Frame timing
			start = glfwGetTime();

			// Process input, changing rotations and zoom
			if (mouseLeft) {
				y_rotation_accel += 0.0005 * (mouseX - pmouseX);
				x_rotation_accel += 0.0005 * (mouseY - pmouseY);
			}
			y_rotation += y_rotation_accel + 0.01 * (keys[GLFW_KEY_LEFT] - keys[GLFW_KEY_RIGHT]);
			x_rotation += x_rotation_accel + 0.01 * (keys[GLFW_KEY_DOWN] - keys[GLFW_KEY_UP]);
			x_rotation = max(x_rotation, -3.14 / 2);
			x_rotation = min(x_rotation, 3.14 / 2);
			y_rotation_accel *= 0.8;
			x_rotation_accel *= 0.8;
			z_rotation_accel *= 0.8;
			zoom *= 1 - mouseScroll * 0.02;

			// Prepare scene rendering, OpenGL stuff
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(Shader);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, HDRI);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, HDRD);

			// Create rotation matrix
			glm::mat4 rotated = glm::rotate(glm::mat4(1), y_rotation, glm::vec3(0, 1, 0));
			rotated = glm::rotate(rotated, x_rotation, glm::vec3(cos(y_rotation), 0, sin(y_rotation)));

			// Create view matrix based on rotation and zoom
			glm::vec3 viewDir = glm::vec4(0, 0, zoom, 0) * rotated;
			uniF3(Shader, "nViewDir", glm::normalize(viewDir));

			// Push transformation, view, and perspective matrices to the shader.
			float ratio = ((float)WINDOW_WIDTH) / ((float)WINDOW_HEIGHT);
			uniM4(Shader, "transform", form(glm::vec3(0), glm::vec3(1)));
			uniM4(Shader, "view", glm::lookAt(viewDir, glm::vec3(0), glm::vec3(0, 1, 0)));
			uniM4(Shader, "perspective", glm::perspective<float>(45.0f, ratio, 0.01f, 10000.0f));

			// Bind and draw the loaded object!
			glBindVertexArray(oVAO);
			glDrawElements(GL_TRIANGLES, obj.faces.size() * 3, GL_UNSIGNED_INT, 0);

			// Use skybox shader, bind skybox sphere model, and render it.
			glUseProgram(SphereShader);
			glBindVertexArray(sVAO);
			uniM4(SphereShader, "transform", form(glm::vec3(0), glm::vec3(zoom*2)));
			uniM4(SphereShader, "view", glm::lookAt(viewDir, glm::vec3(0), glm::vec3(0, 1, 0)));
			uniM4(SphereShader, "perspective", glm::perspective<float>(45.0f, ratio, 0.01f, 10000.0f));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, HDRI);
			glDrawElements(GL_TRIANGLES, sphere.faces.size() * 3, GL_UNSIGNED_INT, 0);

			// Take input, swap buffers (push graphics to screen), and end timing.
			pmouseX = mouseX;
			pmouseY = mouseY;
			mouseScroll *= 0.8;
			glfwPollEvents();
			glfwSwapBuffers(WINDOW);
		}
	}

	glfwTerminate();
	return 0;
}