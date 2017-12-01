#include "Camera.h"

void Camera::updateProjectionView()
{
	m_projectionViewTransform = m_projectionTransform * m_viewTransform;
}

void Camera::updateWorldTransform()
{
	m_worldTransform  = glm::inverse(m_viewTransform);

}

void Camera::setPerspective(float FOV, float AR, float nearPlane, float farPlane)
{
	m_projectionTransform = glm::perspective(FOV, AR, nearPlane, farPlane);
	updateProjectionView();
}
void Camera::setLookAt(const glm::vec3& from,const glm::vec3& to,const glm::vec3& up)
{

	m_viewTransform = glm::lookAt(from,to,up);
	updateProjectionView();
	updateWorldTransform();
}

void Camera::setPositionAndRotation(glm::vec3 position, float yaw, float pitch, float roll)
{
	m_worldTransform = glm::eulerAngleYXZ(yaw, pitch, roll);
	m_worldTransform[3] = glm::vec4(position, 1);//because the matrix is a homogonus it needs a vector 4 and the w component needs to 4 a 1.

	m_viewTransform = glm::inverse(m_worldTransform);
	updateProjectionView();
}

void Camera::getFrustumPlanes(glm::vec4 * planes)
{
	planes[0] = glm::vec4(m_projectionViewTransform[0][3] - m_projectionViewTransform[0][0],
		m_projectionViewTransform[1][3] - m_projectionViewTransform[1][0],
		m_projectionViewTransform[2][3] - m_projectionViewTransform[2][0],
		m_projectionViewTransform[3][3] - m_projectionViewTransform[3][0]);
	
	planes[1] = glm::vec4(m_projectionViewTransform[0][3] + m_projectionViewTransform[0][0],
		m_projectionViewTransform[1][3] + m_projectionViewTransform[1][0],
		m_projectionViewTransform[2][3] + m_projectionViewTransform[2][0],
		m_projectionViewTransform[3][3] + m_projectionViewTransform[3][0]);
	
	planes[2] = glm::vec4(m_projectionViewTransform[0][3] - m_projectionViewTransform[0][1],
		m_projectionViewTransform[1][3] - m_projectionViewTransform[1][1],
		m_projectionViewTransform[2][3] - m_projectionViewTransform[2][1],
		m_projectionViewTransform[3][3] - m_projectionViewTransform[3][1]);
	
	planes[3] = glm::vec4(m_projectionViewTransform[0][3] + m_projectionViewTransform[0][1],
		m_projectionViewTransform[1][3] + m_projectionViewTransform[1][1],
		m_projectionViewTransform[2][3] + m_projectionViewTransform[2][1],
		m_projectionViewTransform[3][3] + m_projectionViewTransform[3][1]);
	
	planes[4] = glm::vec4(m_projectionViewTransform[0][3] - m_projectionViewTransform[0][2],
		m_projectionViewTransform[1][3] - m_projectionViewTransform[1][2],
		m_projectionViewTransform[2][3] - m_projectionViewTransform[2][2],
		m_projectionViewTransform[3][3] - m_projectionViewTransform[3][2]);
	
	planes[5] = glm::vec4(m_projectionViewTransform[0][3] + m_projectionViewTransform[0][2],
		m_projectionViewTransform[1][3] + m_projectionViewTransform[1][2],
		m_projectionViewTransform[2][3] + m_projectionViewTransform[2][2],
		m_projectionViewTransform[3][3] + m_projectionViewTransform[3][2]);

	for (int i = 0; i < 6; i++)
	{
		float d = glm::length(glm::vec3(planes[i]));
		planes[i] /= d;
	}
}

float Camera::getMovementSpeed(void)
{
	return m_moveSpeed;
}

void Camera::setPosition(const glm::vec3& pos)
{
	m_worldTransform[3] = glm::vec4(pos,1);
	m_viewTransform = glm::inverse(m_worldTransform);
	updateProjectionView();
	updateWorldTransform();
}

void Camera::setWorldTransform(const glm::mat4& transform)
{
	m_worldTransform = transform;
	m_viewTransform = glm::inverse(m_worldTransform);
	updateProjectionView();
	updateWorldTransform();
}

glm::mat4 Camera::getWorldTransform()
{
	return m_worldTransform;
}
glm::mat4 Camera::getView()
{
	return m_viewTransform;
}
glm::mat4 Camera::getProjection()
{
	return m_projectionTransform;
}
glm::mat4 Camera::getProjectionView()
{
	return m_projectionViewTransform;
}

glm::vec3 Camera::getPosition()
{
	glm::vec3 pos = getRow(3);
	return pos;
}

glm::vec3 Camera::getRow(unsigned int row)
{
	return m_worldTransform[row].xyz;
}