#include <windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>

#include <string>
#include <memory>
#include <vector>

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
uniform BlobSettings {
	vec4 InnerColor; 
	vec4 OuterColor;
	float RadiusInner;
	float RadiusOuter;
};
void main()
{
	float dx = TexCoord.x - 0.5;
	float dy = TexCoord.y - 0.5;
	float dist = sqrt(dx*dx + dy*dy);
	FragColor = mix(InnerColor, OuterColor, smoothstep(RadiusInner, RadiusOuter, dist));
}
)";

static const char *codeArr[] = { basic_vert };

extern "C" {
	static void opengl_debug_callback__(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

}

void opengl_debug_callback__(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	printf("%s\n", message);
}


static void error_callback(int code, const char *msg)
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

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		RAISE_ERR("create window failed!");
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	ASSERT_MSG(GLEW_OK == err, "Error initializing GLEW: %s\n", glewGetErrorString(err));

	glEnable(GL_DEBUG_OUTPUT);

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
	GLuint vobBuf[GLsizei(VOB_TYPE::MAX)];
	
	V3F_T2F data[] = {
		{ { -0.5f, 0.5f, 0.0f },{ 0.0f, 1.0f } },		// left top
		{ { -0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f } },		// left bottom
		{ { 0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f } },		// right bottom
		{ { 0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f } }		// right top
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);

	glGenBuffers(GLsizei(VOB_TYPE::MAX), vobBuf);

	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[GLsizei(VOB_TYPE::VERTEX_DATA)]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[GLsizei(VOB_TYPE::VERTEX_DATA)]);
	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer(positionLocation,
		sizeof(V3F_T2F::position) / sizeof(float),
		GL_FLOAT, GL_FALSE,
		sizeof(V3F_T2F),
		(const void *)OFFSET_OF(V3F_T2F, position));

	glEnableVertexAttribArray(texCoordLocation);
	glVertexAttribPointer(texCoordLocation, sizeof(V3F_T2F::texCoord) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(V3F_T2F), (const char *)OFFSET_OF(V3F_T2F, texCoord));

	glBindBuffer(GL_ARRAY_BUFFER, vobBuf[GLsizei(VOB_TYPE::UNIFORM_DATA)]);
	{
		GLuint blockIndex = glGetUniformBlockIndex(program, "BlobSettings");
		ASSERT_MSG(blockIndex != GL_INVALID_INDEX, "get index of BlobSettings failed!");
		GLint blockSize;
		glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
		std::unique_ptr<GLubyte> blockBuf(new GLubyte[blockSize]);
		const char *names[] = { "InnerColor", "OuterColor", "RadiusInner", "RadiusOuter" };
		GLuint indices[ARRAY_LENGTH (names)];
		glGetUniformIndices(program, ARRAY_LENGTH (names), names, indices);
		GLint offset[ARRAY_LENGTH (names)];
		glGetActiveUniformsiv(program, ARRAY_LENGTH (names), indices, GL_UNIFORM_OFFSET, offset);
		GLfloat outerColor[] = { 0.1f, 0.0f, 0.0f, 1.0f };
		GLfloat innerColor[] = { 1.0f, 1.0f, 0.75f, 1.0f };
		GLfloat innerRadius = 0.25f, outerRadius = 0.45f;
		memcpy(blockBuf.get() + offset[0], innerColor, sizeof(innerColor));
		memcpy(blockBuf.get() + offset[1], outerColor, sizeof(outerColor));
		memcpy(blockBuf.get() + offset[2], &innerRadius, sizeof(innerRadius));
		memcpy(blockBuf.get() + offset[3], &outerRadius, sizeof(outerRadius));
		glBindBuffer(GL_UNIFORM_BUFFER, vobBuf[GLsizei(VOB_TYPE::UNIFORM_DATA)]);
		glBufferData(GL_UNIFORM_BUFFER, blockSize, blockBuf.get(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, vobBuf[GLsizei(VOB_TYPE::UNIFORM_DATA)]);
	}
	
	GLushort indices[] = {
		0, 1, 2, 2, 3, 0
	};
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, indices);

		{
			GLint debugMessages;
			std::vector<GLchar> messageBuf;
			GLsizei debugMessageSize = 0;
			glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &debugMessages);
			for (GLint i = 0; i < debugMessages; i++) {
				glGetIntegerv(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB, &debugMessageSize);
				messageBuf.reserve(debugMessageSize);
				GLenum source;
				GLenum types;
				GLuint ids;
				GLenum serverities;
				GLsizei lengths;
				glGetDebugMessageLog(1, messageBuf.capacity(), &source, &types, &ids, &serverities, &lengths, messageBuf.data());
				printf("%s\n", messageBuf.data());
			}
			
			
		}
		
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