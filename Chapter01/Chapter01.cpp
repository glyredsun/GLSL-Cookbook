#include <windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>

#include <string>
#include <memory>


static const char *basic_vert = R"(
#version 400
in vec3 VertexPosition;
in vec3 VertexColor;
out vec3 Color;
void main()
{
	Color = VertexColor;
	gl_Position = vec4(VertexPosition, 1.0);
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
		return -1;

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error initializing GLEW: %s\n",
			glewGetErrorString(err));
	}

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
	if (vertShader == 0) {
		fprintf(stderr, "Error creating vertex shader\n");
		return 1;
	}
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
			fprintf(stderr, "Shader log : %s\n", log.get());
		}
	}

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}