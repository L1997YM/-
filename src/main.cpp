#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include "camera.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

const char* glsl_version = "#version 330";
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
//glm::vec3 lightPos(0.2f, 0.5f, 0.5f);

//������ɫ��
const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(aPos,1.0);\n"
"	FragPos = vec3(model * vec4(aPos,1.0));\n"
"	Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"}\0";

const char *vertexShaderSource_g = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"out vec3 LightingColor;\n"
"uniform float reflectance;\n"
"uniform float ambientStrength;\n"
"uniform float specularStrength;\n"
"uniform float diffStrength;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor = vec3(1.0f,1.0f,1.0f);\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(aPos,1.0);\n"
"	vec3 FragPos = vec3(model * vec4(aPos,1.0));\n"
"	vec3 Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"	vec3 ambient = ambientStrength * lightColor;\n"
"	vec3 norm = normalize(Normal);\n"
"   vec3 lightDir = normalize(lightPos - FragPos);\n"
"   float diff = max(dot(norm,lightDir),0.0);\n"
"   vec3 diffuse = diffStrength * diff * lightColor;\n"
"   vec3 viewDir = normalize(viewPos - FragPos);\n"
"   vec3 reflectDir = reflect(-lightDir,norm);\n"
"   float spec = pow(max(dot(viewDir,reflectDir),0.0),reflectance);\n"
"   vec3 specular = specularStrength * spec * lightColor;\n"
"   LightingColor = ambient + diffuse + specular;\n"
"}\0";

const char *lightfragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(1.0);\n"
"}\0";

const char *objectfragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"
"uniform float reflectance;\n"
"uniform float ambientStrength;\n"
"uniform float specularStrength;\n"
"uniform float diffStrength;\n"
"uniform vec3 objectColor = vec3(1.0f,0.5f,0.31f);\n"
"uniform vec3 lightColor = vec3(1.0f,1.0f,1.0f);\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"void main()\n"
"{\n"
"	vec3 ambient = ambientStrength * lightColor;\n"
"	vec3 norm = normalize(Normal);\n"
"   vec3 lightDir = normalize(lightPos - FragPos);\n"
"   float diff = max(dot(norm,lightDir),0.0);\n"
"   vec3 diffuse = diffStrength * diff * lightColor;\n"
"   vec3 viewDir = normalize(viewPos - FragPos);\n"
"   vec3 reflectDir = reflect(-lightDir,norm);\n"
"   float spec = pow(max(dot(viewDir,reflectDir),0.0),reflectance);\n"
"   vec3 specular = specularStrength * spec * lightColor;\n"
"	vec3 result = (ambient + diffuse + specular) * objectColor;\n"
"	FragColor = vec4(result,1.0);\n"
"}\0";

const char *objectfragmentShaderSource_g = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 LightingColor;\n"
"uniform vec3 objectColor = vec3(1.0f,0.5f,0.31f);\n"
"void main()\n"
"{\n"
"	FragColor = vec4(LightingColor * objectColor,1.0);\n"
"}\0";

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);			//����GLFW����Ҫʹ�õ�OpenGL�İ汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);		//����GLFW����ʹ�õ��Ǻ���ģʽ
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);		//����˵����1�����ڵĿ�2�����ڵĸߣ�3����������
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);		//ÿ�����ڵ�����Сʱ����
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))		//��ʼ��GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//����ImGui������
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//����ImGui��ʽ
	ImGui::StyleColorsDark();
	//ƽ̨/��Ⱦ����
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	bool isPhong = false;
	bool isGouraud = false;
	bool bonus = false;
	float ambientStrength = 0.1;
	float diffStrength = 1.0;
	float specularStrength = 0.5;
	float reflectance = 32;

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	unsigned int VBO;
	unsigned int cubeVAO;

	/*----------------------------------------------����VBO--------------------------------------------------*/
	glGenBuffers(1, &VBO);		//����������󣬲���1����Ҫ�����Ļ�������������2�����滺��ID�ĵ�ַ
	glBindBuffer(GL_ARRAY_BUFFER, VBO);		//���������󶨵���Ӧ�Ļ����ϣ�����1���������ͣ�������������or�����������ݣ�
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);		//��vertices���Ƶ���ǰ�󶨻��壬GL_STATIC_DRAW��ʾ���ݼ�������ı�

	/*---------------------------------------------����VAO----------------------------------------------------*/
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	/*-------------------------------------------���Ӷ�������-------------------------------------------------*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);		//����1��Ҫ���õĶ������ԣ�����2���������ԵĴ�С
																						//����3�����ݵ����ͣ�����4������������5��ƫ����
	glEnableVertexAttribArray(0);		//���ö�������
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);		//����1��Ҫ���õĶ������ԣ�����2���������ԵĴ�С
	glEnableVertexAttribArray(0);		//���ö�������
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	while (!glfwWindowShouldClose(window))		//��GLFW�˳�ǰһֱ��������
	{
		if (bonus)
		{
			lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
			lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
		}
		/*---------------------------------------------������ɫ��------------------------------------------------*/
		unsigned int lightvertexShader = glCreateShader(GL_VERTEX_SHADER);		//������ɫ��������Ϊ���ͣ�������ɫ��
		glShaderSource(lightvertexShader, 1, &vertexShaderSource, NULL);		//�����ɫ��Դ��
		glCompileShader(lightvertexShader);		//������ɫ��
		int success;
		char infoLog[512];
		glGetShaderiv(lightvertexShader, GL_COMPILE_STATUS, &success);		//����Ƿ����ɹ�
		if (!success)
		{
			glGetShaderInfoLog(lightvertexShader, 512, NULL, infoLog);		//��ȡ������Ϣ
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		/*--------------------------------------------Ƭ����ɫ��-------------------------------------------------*/
		unsigned int lightfragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(lightfragmentShader, 1, &lightfragmentShaderSource, NULL);
		glCompileShader(lightfragmentShader);
		glGetShaderiv(lightfragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(lightfragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		/*--------------------------------------------��ɫ������-------------------------------------------------*/
		unsigned int lightshaderProgram = glCreateProgram();		//����һ�����򲢷���ID����
		glAttachShader(lightshaderProgram, lightvertexShader);		//����ɫ�����ӵ����������
		glAttachShader(lightshaderProgram, lightfragmentShader);
		glLinkProgram(lightshaderProgram);
		glGetProgramiv(lightshaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(lightshaderProgram, 512, NULL, infoLog);
		}
		glDeleteShader(lightvertexShader);		//ɾ����ɫ������
		glDeleteShader(lightfragmentShader);

		/*---------------------------------------------������ɫ��------------------------------------------------*/
		unsigned int objectvertexShader = glCreateShader(GL_VERTEX_SHADER);		//������ɫ��������Ϊ���ͣ�������ɫ��
		glShaderSource(objectvertexShader, 1, &vertexShaderSource, NULL);		//�����ɫ��Դ��
		glCompileShader(objectvertexShader);		//������ɫ��
		glGetShaderiv(objectvertexShader, GL_COMPILE_STATUS, &success);		//����Ƿ����ɹ�
		if (!success)
		{
			glGetShaderInfoLog(objectvertexShader, 512, NULL, infoLog);		//��ȡ������Ϣ
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		/*--------------------------------------------Ƭ����ɫ��-------------------------------------------------*/
		unsigned int objectfragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(objectfragmentShader, 1, &objectfragmentShaderSource, NULL);
		glCompileShader(objectfragmentShader);
		glGetShaderiv(objectfragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(objectfragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		/*--------------------------------------------��ɫ������-------------------------------------------------*/
		unsigned int objectshaderProgram = glCreateProgram();		//����һ�����򲢷���ID����
		glAttachShader(objectshaderProgram, objectvertexShader);		//����ɫ�����ӵ����������
		glAttachShader(objectshaderProgram, objectfragmentShader);
		glLinkProgram(objectshaderProgram);
		glGetProgramiv(objectshaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(objectshaderProgram, 512, NULL, infoLog);
		}
		glDeleteShader(objectvertexShader);		//ɾ����ɫ������
		glDeleteShader(objectfragmentShader);

		/*---------------------------------------------������ɫ��------------------------------------------------*/
		unsigned int objectvertexShader_g = glCreateShader(GL_VERTEX_SHADER);		//������ɫ��������Ϊ���ͣ�������ɫ��
		glShaderSource(objectvertexShader_g, 1, &vertexShaderSource_g, NULL);		//�����ɫ��Դ��
		glCompileShader(objectvertexShader_g);		//������ɫ��
		glGetShaderiv(objectvertexShader_g, GL_COMPILE_STATUS, &success);		//����Ƿ����ɹ�
		if (!success)
		{
			glGetShaderInfoLog(objectvertexShader_g, 512, NULL, infoLog);		//��ȡ������Ϣ
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		/*--------------------------------------------Ƭ����ɫ��-------------------------------------------------*/
		unsigned int objectfragmentShader_g = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(objectfragmentShader_g, 1, &objectfragmentShaderSource_g, NULL);
		glCompileShader(objectfragmentShader_g);
		glGetShaderiv(objectfragmentShader_g, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(objectfragmentShader_g, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		/*--------------------------------------------��ɫ������-------------------------------------------------*/
		unsigned int objectshaderProgram_g = glCreateProgram();		//����һ�����򲢷���ID����
		glAttachShader(objectshaderProgram_g, objectvertexShader_g);		//����ɫ�����ӵ����������
		glAttachShader(objectshaderProgram_g, objectfragmentShader_g);
		glLinkProgram(objectshaderProgram_g);
		glGetProgramiv(objectshaderProgram_g, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(objectshaderProgram_g, 512, NULL, infoLog);
		}
		glDeleteShader(objectvertexShader_g);		//ɾ����ɫ������
		glDeleteShader(objectfragmentShader_g);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);		//���������Ļ���õ���ɫ
		glClear(GL_COLOR_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		view = glm::lookAt(camera.Position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 model = glm::mat4(1.0f);

		if (isPhong && (!isGouraud))
		{
			glUseProgram(objectshaderProgram);
			unsigned int lightPosLoc = glGetUniformLocation(objectshaderProgram, "lightPos");
			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
			unsigned int viewPosLoc = glGetUniformLocation(objectshaderProgram, "viewPos");
			glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
			unsigned int ambientStrengthPosLoc = glGetUniformLocation(objectshaderProgram, "ambientStrength");
			glUniform1f(ambientStrengthPosLoc, ambientStrength);
			unsigned int diffStrengthPosLoc = glGetUniformLocation(objectshaderProgram, "diffStrength");
			glUniform1f(diffStrengthPosLoc, diffStrength);
			unsigned int specularStrengthPosLoc = glGetUniformLocation(objectshaderProgram, "specularStrength");
			glUniform1f(specularStrengthPosLoc, specularStrength);
			unsigned int reflectancePosLoc = glGetUniformLocation(objectshaderProgram, "reflectance");
			glUniform1f(reflectancePosLoc, reflectance);

			unsigned int projectionLoc = glGetUniformLocation(objectshaderProgram, "projection");
			unsigned int viewLoc = glGetUniformLocation(objectshaderProgram, "view");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

			model = glm::translate(model, glm::vec3(-0.5f, -0.5f, 0.0f));
			unsigned int modelLoc = glGetUniformLocation(objectshaderProgram, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glBindVertexArray(cubeVAO);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			//glDisable(GL_DEPTH_TEST);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		else if ((!isPhong) && isGouraud)
		{
			glUseProgram(objectshaderProgram_g);
			unsigned int lightPosLoc = glGetUniformLocation(objectshaderProgram_g, "lightPos");
			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
			unsigned int viewPosLoc = glGetUniformLocation(objectshaderProgram_g, "viewPos");
			glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
			unsigned int ambientStrengthPosLoc = glGetUniformLocation(objectshaderProgram_g, "ambientStrength");
			glUniform1f(ambientStrengthPosLoc, ambientStrength);
			unsigned int diffStrengthPosLoc = glGetUniformLocation(objectshaderProgram_g, "diffStrength");
			glUniform1f(diffStrengthPosLoc, diffStrength);
			unsigned int specularStrengthPosLoc = glGetUniformLocation(objectshaderProgram_g, "specularStrength");
			glUniform1f(specularStrengthPosLoc, specularStrength);
			unsigned int reflectancePosLoc = glGetUniformLocation(objectshaderProgram_g, "reflectance");
			glUniform1f(reflectancePosLoc, reflectance);

			unsigned int projectionLoc = glGetUniformLocation(objectshaderProgram_g, "projection");
			unsigned int viewLoc = glGetUniformLocation(objectshaderProgram_g, "view");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

			model = glm::translate(model, glm::vec3(-0.5f, -0.5f, 0.0f));
			unsigned int modelLoc = glGetUniformLocation(objectshaderProgram_g, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glBindVertexArray(cubeVAO);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			//glDisable(GL_DEPTH_TEST);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glUseProgram(lightshaderProgram);
		unsigned int objectProjectionLoc = glGetUniformLocation(lightshaderProgram, "projection");
		glUniformMatrix4fv(objectProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		unsigned int objectViewLoc = glGetUniformLocation(lightshaderProgram, "view");
		glUniformMatrix4fv(objectViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.1f));

		unsigned int objectModelLoc = glGetUniformLocation(lightshaderProgram, "model");
		glUniformMatrix4fv(objectModelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//����ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("checkbox");
		ImGui::Checkbox("Phong", &isPhong);
		ImGui::Checkbox("Gouraud", &isGouraud);
		ImGui::Checkbox("bonus", &bonus);
		ImGui::End();

		ImGui::Begin("canshu");
		ImGui::SliderFloat("ambient", (float*)&ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("diffuse", (float*)&diffStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", (float*)&specularStrength, 0.0f, 3.0f);
		ImGui::SliderFloat("reflectance", (float*)&reflectance, 0.0f, 256.0f);
		ImGui::End();

		//glfwPollEvents();		//�����¼������������Ѿ���λ���¼�
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);		//������ɫ����
		glfwPollEvents();		//�����û�д���ʲô�¼�
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();		//�ͷ�ɾ��֮ǰ�����������Դ
	return 0;
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
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
