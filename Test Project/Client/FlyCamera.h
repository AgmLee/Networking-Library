#pragma once

#include "Camera.h"

class FlyCamera: public Camera
{
public:
	virtual void update(float time,GLFWwindow*);

private:
	float m_cameraClock = 0;
	float m_rotateSpeed = .01f;
	double m_oldCursorX;
	double m_oldCursorY;
	void moveCameraUsingKeys(float time, GLFWwindow*);
	void rotateCameraUsingKeys(float time, GLFWwindow*);
	void rotateCameraUsingMouse(float time, GLFWwindow*);
};
