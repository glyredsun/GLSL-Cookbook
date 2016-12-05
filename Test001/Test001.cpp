#include <windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>

#include <string>
#include <memory>

#include <Common.h>


static const char *basic_vert = R"(
#version 400
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexColor;
out vec3 Color;
void main()
{
	Color = VertexColor;
	gl_Position = vec4(VertexPosition, 1.0);
}		
)";

static const char *basic_frag = R"(
#version 400
in vec3 Color;
layout(location = 0) out vec4 FragColor;
void main()
{
	FragColor = vec4(Color, 1.0);
}
)";

static const char *codeArr[] = { basic_vert };

void error_callback(int code, const char *msg)
{
	printf("glfw error : %s\n", msg);
}

int main(void)
{
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	/* Initialize the library */
	if (!glfwInit())
		RAISE_ERR("init glfw failed!");

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		RAISE_ERR("create window failed!");
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	ASSERT_MSG(GLEW_OK == err, "Error initializing GLEW: %s\n", glewGetErrorString(err));

	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("GL Vendor : %s\n", vendor);
	printf("GL Renderer : %s\n", renderer);
	printf("GL Version (string) : %s\n", version);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version : %s\n", glslVersion);

	GLint nExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
	for (GLint i = 0; i < nExtensions; i++) {
		printf("%s\n", glGetStringi(GL_EXTENSIONS, i));
	}

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	ASSERT_MSG(vertShader > 0, "Error creating vertex shader\n");
	glShaderSource(vertShader, 1, &basic_vert, nullptr);
	glCompileShader(vertShader);
	GLint result;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
	if (GL_FALSE == result) {
		GLint logLen;
		glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0) {
			std::unique_ptr<char> log(new char[logLen]);
			GLsizei written;
			glGetShaderInfoLog(vertShader, logLen, &written, log.get());
			RAISE_ERR("Shader log : %s\n", log.get());
		}
	}

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	ASSERT_MSG(fragShader > 0, "Error creating fragment shader");
	glShaderSource(fragShader, 1, &basic_frag, nullptr);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		GLint logLen;
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0) {
			std::unique_ptr<GLchar> logBuf(new GLchar[logLen]);
			GLint wrriten;
			glGetShaderInfoLog(fragShader, logLen, &wrriten, logBuf.get());
			RAISE_ERR("Shader Log : %s\n", logBuf.get());
		}
	}

	GLuint program = glCreateProgram();
	ASSERT_MSG(program > 0, "create program failed!");
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0) {
			std::unique_ptr<GLchar> logBuf(new GLchar[logLen]);
			GLint written;
			glGetProgramInfoLog(program, logLen, &written, logBuf.get());
			RAISE_ERR("Program Log : %s\n", logBuf.get());
		}
	}
	glUseProgram(program);
	GLint colorLocation = glGetAttribLocation(program, "VertexColor");
	ASSERT_MSG(colorLocation >= 0, "VertexColor get failed");
	GLint positionLocation = glGetAttribLocation(program, "VertexPosition");
	ASSERT_MSG(positionLocation >= 0, "VertexPosition get failed");
	enum VOB
	{
		POSITION,
		COLOR,
		MAX
	};
	GLuint vobBuf[2];
	float pos[] = {
		-1, 0, 0,
		1, 0, 1,
		0, 1, 0

	};
	float color[] = {
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	};
	glGenBuffers(MAX, vobBuf);
	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[COLOR]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

	{
		GLint maxLength, nAttribs;
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &nAttribs);
		std::unique_ptr<GLchar> name(new GLchar[maxLength]);
		GLint written, size, location;
		GLenum type;
		for (int i = 0; i < nAttribs; i++) {
			glGetActiveAttrib(program, i, maxLength, &written, &size, &type, name.get());
			location = glGetAttribLocation(program, name.get());
			printf("%-5d | %s\n", location, name);
		}
	}
	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexArrayAttrib(vao, positionLocation);
	//glEnableVertexAttribArray(positionLocation);
	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[POSITION]);
	glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexArrayAttrib(vao, colorLocation);
	//glEnableVertexAttribArray(colorLocation);
	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[COLOR]);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glUseProgram(0);
	glDeleteProgram(program);

	glfwTerminate();
	return 0;
}