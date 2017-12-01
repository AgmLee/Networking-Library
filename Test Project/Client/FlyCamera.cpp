#include "FlyCamera.h"
#include <Application.h>
#include <imgui_glfw3.h>
#include "..\dependencies\glfw\include\GLFW\glfw3.h"
#include <iostream>
using namespace std;
using namespace glm;

void FlyCamera::update(float delta, GLFWwindow* window)
{
	m_cameraClock += delta;
	moveCameraUsingKeys(delta, window);
	rotateCameraUsingKeys(delta, window);
	if (useMouse)
		rotateCameraUsingMouse(delta, window);
}
void FlyCamera::rotateCameraUsingMouse(float time, GLFWwindow* window)
{
	mat4 currentTransform = getWorldTransform();
	currentTransform[3] = vec4(0, 0, 0, 1);
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	double deltaX = m_oldCursorX - cursorX;
	double deltaY = m_oldCursorY- cursorY;
	deltaX = clamp((float)deltaX, -10.0f, 10.0f);
	deltaY = clamp((float)deltaY, -10.0f, 10.0f);
	m_oldCursorX = cursorX;
	m_oldCursorY = cursorY;
	mat4 rotation = glm::rotate((float)deltaX * m_rotateSpeed, vec3(0, 1, 0));
	rotation *= glm::rotate((float)deltaY * m_rotateSpeed, getRow(0));
	currentTransform = rotation * currentTransform;
	currentTransform[3] = vec4(getPosition(), 1);
	setWorldTransform(currentTransform);
}

void FlyCamera::rotateCameraUsingKeys(float delta, GLFWwindow* window)
{
	mat4 currentTransform = getWorldTransform();
	currentTransform[3] = vec4(0, 0, 0, 1);
	mat4 rotation;
	if (glfwGetKey(window, GLFW_KEY_LEFT))
	{
		rotation = glm::rotate(rotation,m_rotateSpeed * keyRotationSpeed, vec3(0, 1, 0));
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT))
	{
		rotation = glm::rotate(rotation, -m_rotateSpeed * keyRotationSpeed, vec3(0, 1, 0));
	}

	if (glfwGetKey(window, GLFW_KEY_UP))
	{
		rotation = glm::rotate(rotation, m_rotateSpeed * keyRotationSpeed, getRow(0));
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN))
	{
		rotation = glm::rotate(rotation, -m_rotateSpeed * keyRotationSpeed, getRow(0));
	}
	currentTransform = rotation * currentTransform;
	currentTransform[3] = vec4(getPosition(), 1);
	setWorldTransform(currentTransform);
}

void FlyCamera::moveCameraUsingKeys(float delta, GLFWwindow* window)
{
	vec3 currentPos = getPosition();
	vec3 left = getRow(0);
	vec3 up = getRow(1);
	vec3 forward = getRow(2);

	if (glfwGetKey(window, GLFW_KEY_W))
	{
		currentPos -= forward* delta * m_moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S))
	{
		currentPos += forward* delta * m_moveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_A))
	{
		currentPos -= left* delta * m_moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D))
	{
		currentPos += left* delta * m_moveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_E))
	{
		currentPos += up* delta * m_moveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_Q))
	{
		currentPos -= up* delta * m_moveSpeed;
	}



	setPosition(currentPos);
}