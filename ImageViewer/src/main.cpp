#include<glad/glad.h>
#include<GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#include<iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void checkShaderErrors(unsigned int shaderID);

int windowWidth = 800;
int windowHeight = 600;


int main(int argc, char* argv[]) {
	//std::cout << argv[0] << argv[1] << std::endl;
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Image Viewer", NULL, NULL);
	if (window == NULL) {
		std::cout << "Fail to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Fail to load GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int tWidth, tHeight, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(argv[1], &tWidth, &tHeight, &nrChannels, 0);
	if (data) {
		std::cout << "texture successfully loaded " << nrChannels << std::endl;
		GLenum format;
		if (nrChannels == 3) format = GL_RGB;
		else format = GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Faild to load texture" << std::endl;
	}
	stbi_image_free(data);


	// vertex shader
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos; \n"
		"layout (location = 1) in vec2 aTexCoord; \n"
		"uniform vec2 windowSize; \n"
		"uniform vec2 imageSize; \n"
		"out vec2 texCoord;\n"
		"void main() \n"
		"{ \n"
		"	vec3 pos = aPos; \n"

		"	gl_Position = vec4(pos, 1.0f); \n"
		"	texCoord = aTexCoord; \n"
		"} \n";

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	checkShaderErrors(vertexShader);

	// fragment shader
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor; \n"
		"in vec2 texCoord;\n"
		"uniform sampler2D tex;\n"
		"void main() \n"
		"{ \n"
		"	FragColor = texture(tex, texCoord); \n"
		"} \n";

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	checkShaderErrors(fragmentShader);

	// program
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	float x, y;
	if (tWidth > tHeight) {
		x = 1.0f;
		y = float(tHeight) / float(tWidth);
		y *= float(windowWidth)/float(windowHeight);
	}
	else {
		y = 1.0f;
		x = float(tWidth) / float(tHeight);
		x *= float(windowHeight)/float(windowWidth);
	}

	float vertices[] = {
		// positions          // texture coords
		 x,  y, 0.0f,   1.0f, 1.0f,   // top right
		 x, -y, 0.0f,   1.0f, 0.0f,   // bottom right
		-x, -y, 0.0f,   0.0f, 0.0f,   // bottom left
		-x,  y, 0.0f,   0.0f, 1.0f    // top left 
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	int windowSizeLocation = glGetUniformLocation(shaderProgram, "windowSize");
	glUniform2f(windowSizeLocation, float(windowWidth), float(windowHeight));

	int imageSizeLocation = glGetUniformLocation(shaderProgram, "imageSize");
	glUniform2f(imageSizeLocation, float(tWidth), float(tHeight));

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.0f, 0.0f, 0.06f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glUseProgram(shaderProgram);
		glUniform2f(windowSizeLocation, float(windowWidth), float(windowHeight));

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);


}


void checkShaderErrors(unsigned int shaderID) {
	int success;
	char infoLog[512];
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
		std::cout << "SHADER::COMPILE::ERROR:: " << infoLog << std::endl;
	}
	else {
		std::cout << "SHADER::COMPILE::SUCCESS" << std::endl;
	}
}