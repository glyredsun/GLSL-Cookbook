#include <windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>

#include <string>
#include <memory>

#include <Common.h>


struct V3F_T2F
{
	glm::vec3 position;
	glm::vec2 texCoord;
}; 

static const char *basic_vert = R"(
#version 400
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexTexCoord;
out vec2 TexCoord;
void main()
{
	TexCoord = VertexTexCoord;
	gl_Position = vec4(VertexPosition, 1.0);
}		
)";

static const char *basic_frag = R"(
#version 400
in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;
//uniform BlobSettings {
	vec4 InnerColor = vec4(1.0f, 1.0f, 0.75f, 1.0f); 
	vec4 OuterColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	float RadiusInner = 0.25f;
	float RadiusOuter = 0.45f;
//};
void main()
{
	float dx = TexCoord.x - 0.5;
	float dy = TexCoord.y - 0.5;
	float dist = sqrt(dx*dx + dy*dy);
	FragColor = mix(InnerColor, OuterColor, smoothstep(RadiusInner, RadiusOuter, dist));
	FragColor = vec4(FragColor.r + 0.5, FragColor.g + 0.5, FragColor.b + 0.5, 1.0);
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
	GLint texCoordLocation = glGetAttribLocation(program, "VertexTexCoord");
	ASSERT_MSG(texCoordLocation >= 0, "VertexTexCoord get failed");
	GLint positionLocation = glGetAttribLocation(program, "VertexPosition");
	ASSERT_MSG(positionLocation >= 0, "VertexPosition get failed");
	enum class VOB_TYPE
	{
		VERTEX_DATA,
		UNIFORM_DATA,
		MAX
	};
	GLuint vobBuf[2];
	
	V3F_T2F data[] = {
		{ { -0.5f, 0.5f, 0.0f },{ 0.0f, 1.0f } },		// left top
		{ { -0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f } },		// left bottom
		{ { 0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f } },		// right bottom
		{ { 0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f } }		// right top
	};
	glGenBuffers(GLsizei(VOB_TYPE::MAX), vobBuf);
	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[GLsizei(VOB_TYPE::VERTEX_DATA)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, vobBuf[GLsizei(VOB_TYPE::UNIFORM_DATA)]);
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[GLsizei(VOB_TYPE::VERTEX_DATA)]);
	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer(positionLocation,
		sizeof(V3F_T2F::position) / sizeof(float),
		GL_FLOAT, GL_FALSE,
		sizeof(V3F_T2F) - sizeof(V3F_T2F::position),
		(const void *)OFFSET_OF(V3F_T2F, position));
	glEnableVertexAttribArray(texCoordLocation);
	glVertexAttribPointer(texCoordLocation, sizeof(V3F_T2F::texCoord) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(V3F_T2F) - sizeof(V3F_T2F::texCoord), (const char *)OFFSET_OF(V3F_T2F, texCoord));
	
	GLushort indices[] = {
		1, 2, 3, 3, 4, 1
	};
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, indices);

		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glUseProgram(0);
	glDeleteProgram(program);
	glDeleteBuffers(GLsizei(VOB_TYPE::MAX), vobBuf);
	
	glfwTerminate();
	return 0;
}