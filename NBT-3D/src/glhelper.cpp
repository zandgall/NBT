#include "glhelper.h"

GLFWwindow* WINDOW;
int WINDOW_WIDTH, WINDOW_HEIGHT;

double mouseX, mouseY, pmouseX, pmouseY, mouseScroll;
bool mouseLeft, mouseRight, mouseMiddle;

bool* keys = new bool[GLFW_KEY_LAST];

bool USE_TEXTURE = false;

std::map<std::string, unsigned int> CACHE = std::map<std::string, unsigned int>();

void framebuffer_size_callback(GLFWwindow* wnd, int width, int height) {
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;
	glfwSetWindowSize(wnd, width, height);
}

void keypress_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	keys[key] = action == GLFW_PRESS || action == GLFW_REPEAT;
}

void mouse_scroll_call(GLFWwindow* window, double x, double y) {
	mouseScroll = y;
}

void mouse_move_call(GLFWwindow* window, double x, double y) {
	mouseX = x;
	mouseY = y;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		mouseLeft = action == GLFW_PRESS;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		mouseRight = action == GLFW_PRESS;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		mouseMiddle = action == GLFW_PRESS;
		break;
	}
}

/// <summary>
/// Initiates OpenGL through GLFW
/// </summary>
/// <param name="verMajor">Major part of version number</param>
/// <param name="verMinor">Minor part of version number</param>
/// <param name="name">Name of the window to be created</param>
/// <param name="windowWidth">Width of window to be created</param>
/// <param name="windowHeight">Height of window to be created</param>
/// <returns>0 if success, -1 if error</returns>
int START_OPEN_GL(char verMajor, char verMinor, const char* name, int windowWidth, int windowHeight) {
	// Initiate
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, verMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, verMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window and use it
	WINDOW = glfwCreateWindow(windowWidth, windowHeight, name, NULL, NULL);
	WINDOW_WIDTH = windowWidth;
	WINDOW_HEIGHT = windowHeight;
	if (WINDOW == NULL) {
		//std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(WINDOW);

	// Callbacks
	glfwSetFramebufferSizeCallback(WINDOW, framebuffer_size_callback);
	glfwSetCursorPosCallback(WINDOW, mouse_move_call);
	glfwSetMouseButtonCallback(WINDOW, mouse_button_callback);
	glfwSetKeyCallback(WINDOW, keypress_callback);
	glfwSetScrollCallback(WINDOW, mouse_scroll_call);

	// Load opengl extensions via glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Glad loading failed!" << std::endl;
		return -1;
	}

	for(int i = 0; i < GLFW_KEY_LAST; i++)
		keys[i] = false;

	return 0;
}

void uniM4(unsigned int shader, const GLchar* address, glm::mat4 matrix) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniformMatrix4fv(vertexColorLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}
void uniF4(unsigned int shader, const GLchar* address, glm::vec4 vec) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform4f(vertexColorLocation, vec.x, vec.y, vec.z, vec.w);
}
void uniF3(unsigned int shader, const GLchar* address, glm::vec3 vec) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform3f(vertexColorLocation, vec.x, vec.y, vec.z);
}
void uniF2(unsigned int shader, const GLchar* address, glm::vec2 vec) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform2f(vertexColorLocation, vec.x, vec.y);
}
void uniF1(unsigned int shader, const GLchar* address, float a) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform1f(vertexColorLocation, a);
}
void uniF4(unsigned int shader, const GLchar* address, float a, float b, float c, float d) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform4f(vertexColorLocation, a, b, c, d);
}
void uniF3(unsigned int shader, const GLchar* address, float a, float b, float c) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform3f(vertexColorLocation, a, b, c);
}
void uniF2(unsigned int shader, const GLchar* address, float a, float b) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform2f(vertexColorLocation, a, b);
}
void uniI4(unsigned int shader, const GLchar* address, int a, int b, int c, int d) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform4i(vertexColorLocation, a, b, c, d);
}
void uniI3(unsigned int shader, const GLchar* address, int a, int b, int c) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform3i(vertexColorLocation, a, b, c);
}
void uniI2(unsigned int shader, const GLchar* address, int a, int b) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform2i(vertexColorLocation, a, b);
} 
void uniI1(unsigned int shader, const GLchar* address, int a) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUniform1i(vertexColorLocation, a);
}
void uniB(unsigned int shader, const GLchar* address, bool a) {
	int vertexColorLocation = glGetUniformLocation(shader, address);
	glUseProgram(shader); 
	glUniform1i(vertexColorLocation, a);
}

// Returns a translation matrix
glm::mat4 translation(glm::vec3 pos) {
	return glm::translate(glm::mat4(1.0), pos);
}
// Returns a rotation matrix off of a given matrix
glm::mat4 rotation(glm::mat4 mat, glm::vec3 rot) {
	mat = glm::rotate(mat, rot.x, glm::vec3(1.0, 0.0, 0.0));
	mat = glm::rotate(mat, rot.y, glm::vec3(0.0, 1.0, 0.0));
	mat = glm::rotate(mat, rot.z, glm::vec3(0.0, 0.0, 1.0));
	return mat;
}
// Returns a rotation matrix off of [1]
glm::mat4 rotation(glm::vec3 rot) {
	glm::mat4 out = glm::mat4(1.0);
	out = glm::rotate(out, rot.y, glm::vec3(0.0, 1.0, 0.0));
	out = glm::rotate(out, rot.x, glm::vec3(1.0, 0.0, 0.0));
	out = glm::rotate(out, rot.z, glm::vec3(0.0, 0.0, 1.0));
	return out;
}
// Returns a scalar matrix
glm::mat4 scaled(glm::vec3 scaling) {
	glm::mat4 out = glm::mat4(1.0);
	out = glm::scale(out, scaling);
	return out;
}
// Returns a positioned, rotated, and possibly scaled matrix off of given matrix, or [1]
glm::mat4 form(glm::vec3 pos, glm::vec3 scale, glm::vec3 rot, glm::mat4 mat) {
	return glm::scale(rotation(glm::translate(mat, pos), rot), scale);
}
// Returns a transform function for a specified rectangle shape
glm::mat4 rect(float x, float y, float w, float h, glm::vec3 rot, glm::mat4 mat) {
	return form(glm::vec3(x + w / 2.f, y + h / 2.f, 0), glm::vec3(w / 2.f, h / 2.f, 0), rot, mat);
}

unsigned int CompileShader(unsigned int type, const std::string& source) {

	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

		char* message = (char*)alloca(length * sizeof(char));

		glGetShaderInfoLog(id, length, &length, message);

		std::string line;
		std::istringstream stream(source);
		int lineNum = 0;
		while (std::getline(stream, line)) {
			std::cout << ++lineNum << ": " << line << std::endl;
		}

		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

unsigned int loadShader(const char* filepath) {
	//if (CACHE.find(filepath) != CACHE.end())
	//	return CACHE[filepath];
	std::ifstream stream(filepath);

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	int lineNum = 1;
	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
			lineNum = 1;
		}
		else {
			//std::cout << lineNum << " : " << line << std::endl;
			lineNum++;
			ss[(int)type] << line << '\n';
		}
	}

	stream.clear();
	stream.close();	

	unsigned int program = glCreateProgram();

	unsigned int vs = 0;
	vs = CompileShader(GL_VERTEX_SHADER, ss[0].str());
	unsigned int fs = 0;
	fs = CompileShader(GL_FRAGMENT_SHADER, ss[1].str());

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	//CACHE.insert(std::make_pair(filepath, program));
	return program;
}

unsigned int getTextureFrom(const char* filepath, int magFilter, int minFilter, int wrapS, int wrapT) {
	//if (CACHE.find(filepath) != CACHE.end())
	//	return CACHE[filepath];
	unsigned int texture;						// Have empty number,
	glGenTextures(1, &texture);					// Gen texture id into that number,
	glBindTexture(GL_TEXTURE_2D, texture);		// Bind it

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);			// Repeat/clip/etc image horizontally
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);			// Repeat/clip/etc image vertically
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);	// Set interpolation at minimum
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);	// Set interpolation at maximum

	int width, height, nrChannels;					// Have empty width, height and channel numbers
	std::string string = std::string(filepath);		// Load the chars into a string
	int loadtype = STBI_rgb;						// Set loadtype default to RGB
	if (string.find(".png") != std::string::npos)	// If it's a png use RGBA									(COULD BE AN ISSUE WITH 24 BIT PNGs)
		loadtype = STBI_rgb_alpha;
	//stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, STBI_rgb_alpha); // Load the texture and get width, height, and channel number
	if (data) {
		//unsigned type = loadtype == STBI_rgb ? GL_RGB : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);	// Buffer data into the texture
		glGenerateMipmap(GL_TEXTURE_2D);														// Generate mipmap based on interpolation
	}
	else {
		std::cout << "Failed to load texture " << filepath << std::endl;	// Scream if needed
		return 0;
	}
	stbi_image_free(data);					// Empty data,
	//CACHE.insert(std::make_pair(filepath, texture));
	return texture;							// and return the texture
}


unsigned int getHDRTexture(const char* filepath, int magFilter, int minFilter, int wrapS, int wrapT) {
	unsigned int texture;						// Have empty number,
	glGenTextures(1, &texture);					// Gen texture id into that number,
	glBindTexture(GL_TEXTURE_2D, texture);		// Bind it

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);			// Repeat/clip/etc image horizontally
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);			// Repeat/clip/etc image vertically
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);	// Set interpolation at minimum
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);	// Set interpolation at maximum

	int width, height, nrChannels;					// Have empty width, height and channel numbers
	std::string string = std::string(filepath);		// Load the chars into a string
	int loadtype = STBI_rgb;						// Set loadtype default to RGB
	if (string.find(".png") != std::string::npos)	// If it's a png use RGBA									(COULD BE AN ISSUE WITH 24 BIT PNGs)
		loadtype = STBI_rgb_alpha;
	//stbi_set_flip_vertically_on_load(true);

	float* data = stbi_loadf(filepath, &width, &height, &nrChannels, STBI_rgb_alpha); // Load the texture and get width, height, and channel number
	if (data) {
		//unsigned type = loadtype == STBI_rgb ? GL_RGB : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);	// Buffer data into the texture
		//glGenerateMipmap(GL_TEXTURE_2D);														// Generate mipmap based on interpolation
	}
	else {
		std::cout << "Failed to load texture " << filepath << std::endl;	// Scream if needed
		return 0;
	}
	stbi_image_free(data);					// Empty data,
	return texture;							// and return the texture
}