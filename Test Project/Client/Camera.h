#pragma once
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct GLFWwindow;

class Camera
{
private:
	glm::mat4 m_worldTransform;
	glm::mat4 m_viewTransform;
	glm::mat4 m_projectionTransform;
	glm::mat4 m_projectionViewTransform;
	void updateProjectionView();
	void updateWorldTransform();
public:
	virtual void update(float deltaTime, GLFWwindow* window) = 0;
	void setPerspective(float FOV,float AR,float nearPlane,float farPlane);
	void setLookAt(const glm::vec3& from,const glm::vec3& to,const glm::vec3& up);
	void setPosition(const glm::vec3& pos);
	void setWorldTransform(const glm::mat4& transform);
	void setPositionAndRotation(glm::vec3 position, float yaw, float pitch,float roll);
	void getFrustumPlanes(glm::vec4* planes);
	float getMovementSpeed(void);
	glm::mat4 getWorldTransform();
	glm::mat4 getView();
	glm::mat4 getProjection();
	glm::mat4 getProjectionView();
	glm::vec3 getPosition();
	glm::vec3 getRow(unsigned int row);
	bool useMouse = true;
	float keyRotationSpeed = 1.0f;
	float m_moveSpeed = 10;
};