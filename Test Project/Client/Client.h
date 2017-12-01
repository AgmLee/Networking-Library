#pragma once

#include "Application.h"
#include <glm/mat4x4.hpp>
#include <vector>

class ClientControl;
class Camera;

class Client : public aie::Application {
public:

	Client();
	virtual ~Client();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

protected:
	ClientControl* m_client;
	Camera* m_cam;
};